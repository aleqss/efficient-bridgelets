#!/usr/bin/env python3

# Copyright 2023, 2024, 2025 Aleksandr Popov
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version. This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details. You should have received a copy of the GNU
# General Public License along with this program. If not, see
# <https://www.gnu.org/licenses/>.

'''
Download the OpenPFLOW data, unpack it, and process it into a suitable format.

Change `get_unpack_data()` if the URL needs to change. Processing consists of
projecting lat/lon coordinates to UTM zone 54 and discretising them; time
stamps are made relative to the start of each trajectory and discretised. A
trajectory is split based on mode changes. All of the trajectories are written
in a simple format, one trajectory per file, together with `train.txt` and
`test.txt` files listing them. Run `get_data.py -h` for options.
'''

import argparse
import io
import multiprocessing
import pathlib
import random
import shutil
import sys
import urllib.request
import zipfile
import zlib
from dataclasses import dataclass, field
from typing import Any

try:
    import cartopy.crs  # type: ignore
    import numpy as np
    import pandas
except ImportError as err:
    print(err)
    print('Please install numpy, pandas, and cartopy.\nSee requirements.txt.')
    sys.exit(1)


@dataclass(eq=False, kw_only=True)
class conf:  # pylint: disable=C0103
    '''Global config.'''
    verbose: bool = False
    fill_in: bool = True
    dense: bool = False
    diagonal: bool = False
    straight: set[str] = field(default_factory=set)


def vprint(*args: Any, **kwargs: Any) -> Any:
    '''A simple logging wrapper, passing `args` and `kwargs` to `print()`.'''
    if conf.verbose:
        print(*args, **kwargs)


# -----------------------------------------------------------------------------
# INTERFACE
# -----------------------------------------------------------------------------
def parse_args() -> argparse.Namespace:
    '''Parse the arguments to the script, run with -h for help.'''
    parser = argparse.ArgumentParser(
        description='download and prepare OpenPFLOW data for the experiments',
        epilog='further links and explanation at '
               'https://github.com/aleqss/efficient-bridgelets'
    )
    parser.add_argument('subdir', nargs='?', type=pathlib.Path,
                        default=pathlib.Path(sys.argv[0]).resolve().parent,
                        help='store the data in a given directory')
    acts = parser.add_mutually_exclusive_group()
    acts.add_argument('-d', '--only-download', action='store_true',
                      default=False, help='skip processing the data')
    acts.add_argument('-p', '--only-process', action='store_true',
                      default=False,
                      help='process already available trajectories.tsv')
    parser.add_argument('-m', '--straight', action='extend', nargs=1,
                        choices=['si', 'dl'], help='estimate straightness for '
                        'each trajectory using sinuosity (si) or maximum '
                        'distance from the line between the endpoints (dl), '
                        'use this option twice if needed')
    parser.add_argument('-g', '--use-diagonals', action='store_true',
                        default=False,
                        help='assume the model allows diagonal movement')
    parser.add_argument('-k', '--keep-original', action='store_true',
                        default=False,
                        help='do not attempt to fill in assured measurements')
    prec = parser.add_mutually_exclusive_group()
    prec.add_argument('-f', '--fine', action='store_true', default=False,
                      help='use a fine grid, more suitable for applications')
    prec.add_argument('-s', '--search', action='store_true', default=False,
                      help='search for good discretisation parameters')
    parser.add_argument('-z', '--zoom-out', action='store_true', default=False,
                        help='discretise so that all the data is dense')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='show progress information')
    parser.add_argument('--version', action='version', version='%(prog)s 1.0')
    args = parser.parse_args()

    if not args.subdir.is_dir():
        print(f'error: {args.subdir} is not an existing directory')
        parser.print_help()
        sys.exit(1)

    if args.only_download:
        print('only downloading, all options except -v/--verbose ignored')
    elif args.zoom_out and not args.search:
        print('not searching for discretisation parameters, -z/--zoom-out'
              'ignored')
    return args


# -----------------------------------------------------------------------------
# DOWNLOAD
# -----------------------------------------------------------------------------
def download(part: str) -> str:
    '''Download a partial archive and extract it.'''
    with urllib.request.urlopen(part) as sin:
        with zipfile.ZipFile(io.BytesIO(sin.read())) as archive:
            return archive.extract(archive.infolist()[0])


def get_unpack_data(subdir: pathlib.Path) -> None:
    '''Download the data, extract it, and concatenate the files in `subdir`.'''
    baseurl = ('https://open-research-data.s3.ap-northeast-1.amazonaws.com/'
               'openpflow/trajectory0')
    parts = (baseurl + str(i) + '.tsv.zip' for i in range(1, 10))

    vprint('downloading files')
    with multiprocessing.Pool() as p:
        files = p.map(download, parts)

    vprint('unpacking to the dataset file')
    with open(subdir / 'trajectories.tsv', 'wb') as comp:
        for file in files:
            with open(file, 'rb') as inp:
                shutil.copyfileobj(inp, comp)
    for file in files:
        pathlib.Path(file).unlink()

    vprint('verifying correctness')
    crcv = 0
    with open(subdir / 'trajectories.tsv', 'rb') as verf:
        while buf := verf.read(1024 * 1024 * 256):
            crcv = zlib.crc32(buf, crcv)
    assert crcv == 0x21ff3588, 'Download failed, please retry.'


# -----------------------------------------------------------------------------
# BASIC PROCESSING
# -----------------------------------------------------------------------------
def _diag_length(x1: int, x2: int, y1: int, y2: int) -> int:
    '''Compute the shortest path length allowing diagonal movement.'''
    return max(abs(x2 - x1), abs(y2 - y1))


def _rect_length(x1: int, x2: int, y1: int, y2: int) -> int:
    '''Compute the shortest path length when only going through neighbours.'''
    return abs(x2 - x1) + abs(y2 - y1)


def _last_valid(buf: list[list[int]], last: list[int]) -> list[int]:
    '''Find the last element in `buf` that is valid w.r.t. `last`.'''
    assert len(last) == 3, 'last should be a single (t, x, y) measurement'
    tb, xb, yb = last
    dist = _diag_length if conf.diagonal else _rect_length
    for t, x, y in reversed(buf):
        if dist(xb, x, yb, y) <= t - tb:
            return [t, x, y]
    return []


def resolve_dups(traj: pandas.DataFrame) -> pandas.DataFrame:
    '''
    Resolve duplicates by picking the last valid one w.r.t. already seen.

    Given a (t, x, y) dataframe `traj`, we process it in order, resolving
    multiple measurements at the same timestamp t.
    '''

    buffer: list[list[int]] = []
    dedup: list[list[int]] = []

    def select_last() -> list[int]:
        '''Pick the last valid measurement from `buffer` to follow `dedup`.'''
        e = _last_valid(buffer, dedup[-1]) if dedup else buffer[-1]
        dedup.append(e)
        return e

    for t, x, y in traj.itertuples(index=False):
        if not buffer:
            buffer = [[t, x, y]]
            continue
        if buffer[0][0] < t:
            e = select_last()
            if not e:
                return pandas.DataFrame(columns=['t', 'x', 'y'])
            buffer = [[t, x, y]]
        else:
            buffer.append([t, x, y])

    e = select_last()
    if not e:
        return pandas.DataFrame(columns=['t', 'x', 'y'])
    return pandas.DataFrame(dedup, columns=['t', 'x', 'y'])


def interpolate(a: list[int], b: list[int]) -> list[list[int]]:
    '''Interpolate between a = [ta, xa, ya] and b = [tb, xb, yb].'''
    ta, xa, ya = a
    tb, xb, yb = b
    assert abs(xb - xa) + abs(yb - ya) == tb - ta and (yb == ya or xb == xa), \
           'cannot interpolate'
    va, vb = (xa, xb) if abs(xb - xa) == tb - ta else (ya, yb)
    genv = range(va, vb + 1) if va < vb else range(va, vb - 1, -1)
    gent = range(ta, tb + 1)
    res = [list(gent), list(genv), [ya] * (tb + 1 - ta)] if ya == yb else \
          [list(gent), [xa] * (tb + 1 - ta), list(genv)]
    return [[r[i] for r in res] for i in range(tb - ta + 1)]


def fill_gaps(df: pandas.DataFrame) -> tuple[pandas.DataFrame, bool]:
    '''
    Attempt to fill in easy gaps, if impossible return unchanged.

    An easy gap is one between (t, x, y) and (t + k, x + k, y), or
    symmetric with +/- and x/y. In this scenario, the model allows exactly
    one sequence of steps. Return either the original dataframe or a dense
    one if successful, and whether it has been made dense.
    '''

    res: list[list[int]] = []
    for t, x, y in df.itertuples(index=False):
        if not res:
            res = [[t, x, y]]
            continue
        tp, xp, yp = res[-1]
        if tp < t - 1:
            if t - tp == abs(x - xp) + abs(y - yp) and (x == xp or y == yp):
                res += interpolate([tp, xp, yp], [t, x, y])[1:]
            else:
                return df, False
        else:
            res.append([t, x, y])
    return pandas.DataFrame(res, columns=['t', 'x', 'y']), True


def test_valid(traj: pandas.DataFrame) -> tuple[pandas.DataFrame, bool]:
    '''
    Check that the trajectory is valid and measured densely.

    A trajectory is valid if:
    * between every pair of measurements, the Manhattan distance is at most
      the time difference;
    * and there is at most one measurement per time step.

    First, resolve duplicates by picking the last valid measurement.
    Side effect: we know that all steps are valid.
    Then, either return or attempt to fill in gaps in density.
    '''

    assert traj.columns.to_list() == ['t', 'x', 'y'] and traj.shape[0] > 2, \
           'trajectory has incorrect shape'

    res = traj.sort_values(by='t', kind='stable', ignore_index=True)
    res = resolve_dups(res)
    dense = res['t'].diff().iloc[1:].eq(1).all()
    if res.empty or dense or not conf.fill_in:
        return res, dense
    return fill_gaps(res)


def straightness(df: pandas.DataFrame) -> float:
    '''Estimate straightness by maximum deviation from the straight line.'''
    dfxy = df[['easting', 'northing']].copy()
    x1, y1 = dfxy.iloc[0]
    x2, y2 = dfxy.iloc[-1]

    if x1 == x2 and y1 == y2:
        dfxy['xa'] = dfxy['easting'] - x1
        dfxy['ya'] = dfxy['northing'] - y1
        return dfxy[['xa', 'ya']].apply(lambda r: np.hypot(*r), axis=1).max()

    dx, dy = x2 - x1, y2 - y1
    add = x2 * y1 - y2 * x1
    length = np.hypot(dx, dy)

    def dist(x0: float, y0: float) -> float:
        '''Distance between a point (x0, y0) and the line.'''
        return np.abs(dy * x0 - dx * y0 + add) / length

    return dfxy.apply(lambda r: dist(*r), axis=1).max()


def sinuosity(df: pandas.DataFrame) -> float:
    '''Estimate straightness by sinuosity (path length / straight length).'''
    dfxy = df[['easting', 'northing']]
    x1, y1 = dfxy.iloc[0]
    x2, y2 = dfxy.iloc[-1]

    if x1 == x2 and y1 == y2:
        return np.inf

    length = np.hypot(x2 - x1, y2 - y1)
    pl = dfxy.diff().iloc[1:].apply(lambda r: np.hypot(*r), axis=1).sum()
    return pl / length


def _write_tr(pid: int, pnum: int, fname: int, tr: pandas.DataFrame,
              cd: pathlib.Path) -> None:
    '''Output a discretised trajectory to a file.'''
    ftemp = 'pid = {}, pnum = {}\n\n{}'
    with open(cd / str(fname), 'w', encoding='utf-8') as cfile:
        cfile.write(
            ftemp.format(
                pid, pnum, tr.to_csv(index=False, columns=['t', 'x', 'y'])
            )
        )


def _write_str(strdev: pandas.DataFrame, cd: pathlib.Path) -> None:
    '''Output the straightness values to a file.'''
    to_wr = ['fname'] + sorted(list(conf.straight))
    strdev.to_csv(cd / 'straightness', sep=' ', columns=to_wr, index=False,
                  encoding='utf-8', lineterminator='\n')


def split_count(df: pandas.DataFrame, cur_dir: pathlib.Path,
                write_out: bool = False) -> tuple[list[int], list[int], int]:
    '''Count how well we do (and output stuff).'''
    if write_out:
        cur_dir.mkdir(mode=0o755, exist_ok=True)

    sparse, dense = [], []
    fname = 0
    strdev: list[tuple[int, float, float]] = []

    for (pid, pnum), gr in df.groupby(['pid', 'pnum'], sort=False):
        if fname > 0 and fname % 100000 == 0:
            vprint(f'processed {fname} trajectories')

        if gr.shape[0] > 2:
            if conf.straight:
                strdev.append((fname, straightness(gr), sinuosity(gr)))
            c, d = test_valid(gr[['t', 'x', 'y']])
            if c.shape[0] > 2 and d:
                dense.append(fname)
            elif c.shape[0] > 2:
                sparse.append(fname)

            if write_out:
                _write_tr(pid, pnum, fname, c, cur_dir)
            fname += 1

    if write_out and conf.straight:
        _write_str(pandas.DataFrame(strdev, columns=['fname', 'dl', 'si']),
                   cur_dir)

    vprint(f'processed {fname} trajectories')
    return sparse, dense, fname


# -----------------------------------------------------------------------------
# TRIAL
# -----------------------------------------------------------------------------
def _est_dx(dt: int, speed: int, ratio: float = 3.6) -> int:
    '''Find a good ratio for the distance, so we do not skip over.'''
    return max(int(speed * dt / ratio), 1)


def _trial_discr(res: pandas.DataFrame, cur_dir: pathlib.Path, deltax: int,
                 deltat: int) -> tuple[list[int], list[int], int]:
    '''Discretise and count, helper function.'''
    res['x'] = (res['easting'] // deltax).astype('int64')
    res['y'] = (res['northing'] // deltax).astype('int64')
    grouped = res.groupby(['pid', 'pnum'], sort=False)
    res['t'] = (
        (res['time'] - grouped['time'].transform('min')).dt.total_seconds()
        // deltat).astype('int64')
    sp, dn, tot = split_count(res, cur_dir)
    vprint(f'Total: {tot}, sparse: {len(sp)}, dense: {len(dn)}')
    vprint(f'{(len(sp) + len(dn)) / tot:.2%} valid, '
           f'of which {len(dn) / (len(sp) + len(dn)):.2%} dense')
    return sp, dn, tot


def find_split(
        df: pandas.DataFrame, cur_dir: pathlib.Path, max_speed: int,
        ratio: float = 3.6, goal_dense: int = 15) -> tuple[int, int, float]:
    '''Discretise appropriately for a single mode.'''
    deltat = 1
    deltax = _est_dx(deltat, max_speed, ratio)
    valid = 0.0

    res = df.copy()
    while True:
        vprint(f'trying dt = {deltat}, dx = {deltax}')
        sp, dn, tot = _trial_discr(res, cur_dir, deltax, deltat)
        valid = (len(sp) + len(dn)) / tot
        if 100 * len(dn) >= goal_dense * (len(dn) + len(sp)):
            break
        deltat = 2 * deltat
        deltax = _est_dx(deltat, max_speed, ratio)

    if deltat > 4:
        lt, rt = deltat // 2, deltat
        deltat = (lt + rt) // 2
        deltax = _est_dx(deltat, max_speed, ratio)
        while lt < deltat < rt:
            vprint(f'trying dt = {deltat}, dx = {deltax}')
            sp, dn, tot = _trial_discr(res, cur_dir, deltax, deltat)
            if 100 * len(dn) >= goal_dense * (len(dn) + len(sp)):
                rt = deltat
                valid = (len(sp) + len(dn)) / tot
            else:
                lt = deltat
            deltat = (lt + rt) // 2
            deltax = _est_dx(deltat, max_speed, ratio)
        deltat = rt
        deltax = _est_dx(deltat, max_speed, ratio)
    return deltat, deltax, valid


def finetune(df: pandas.DataFrame, subdir: pathlib.Path,
             goal_dense: int = 15) -> tuple[dict[int, int], dict[int, int]]:
    '''Find the appropriate discretisation boundaries.'''
    speeds = {1: 5, 2: 87, 3: 125, 4: 15}  # ~99th percentile
    deltat, deltax = {99: 1}, {99: 1}
    df['pnum'] = df['mode'].ne(df['mode'].shift()).cumsum()
    for mode in [1, 2, 3, 4]:
        vprint(f'trial for mode {mode}')
        current = df.groupby('mode').get_group(mode)
        best_v, best_r = 0.0, 0.0
        for ratio in [4.2, 3.6, 3.0, 2.4, 1.8, 1.2]:
            dt, dx, v = find_split(current, subdir / str(mode), speeds[mode],
                                   ratio, goal_dense)
            if v > best_v:
                deltat[mode] = dt
                deltax[mode] = dx
                best_v, best_r = v, ratio
            if best_v > 0.9:
                break
        vprint(f'at least 90% valid in mode {mode}: {best_v:.2%} '
               f'with ratio {best_r}, {dt=}, {dx=}')
    return deltat, deltax


# -----------------------------------------------------------------------------
# FINAL
# -----------------------------------------------------------------------------
def project_data(subdir: pathlib.Path) -> pandas.DataFrame:
    '''Load the TSV file and project lat/lon to UTM zone 54.'''
    # Load the TSV file into a pandas dataframe
    vprint('reading dataframe')
    df = pandas.read_csv(subdir / 'trajectories.tsv', sep='\t', header=None,
                         names=['pid', 'time', 'lon', 'lat', 'mode'])
    df['time'] = pandas.to_datetime(df['time'], format="%Y-%m-%d %H:%M:%S")

    # Convert lon/lat to UTM zone 54 eastings/northings
    utmj = cartopy.crs.UTM(54)
    df[['easting', 'northing', 'z_dummy']] = \
        utmj.transform_points(cartopy.crs.PlateCarree(), df['lon'], df['lat'])
    df.drop(columns=['z_dummy', 'lon', 'lat'], inplace=True)
    return df


def discretise(df: pandas.DataFrame, deltat: dict[int, int],
               deltax: dict[int, int]) -> pandas.DataFrame:
    '''Discretise the measurements according to mode.'''
    res = df.copy()
    # Discretise eastings/northings to x/y
    res['x'] = (res['easting'] // res['mode'].map(deltax)).astype('int64')
    res['y'] = (res['northing'] // res['mode'].map(deltax)).astype('int64')

    # Group by shifts in mode for each pid, compute time offset from start,
    # discretise the time steps
    res['dt'] = res['mode'].map(deltat)
    res['pnum'] = res['mode'].ne(res['mode'].shift()).cumsum()
    grouped = res.groupby(['pid', 'pnum'], sort=False)
    res['t'] = (
        (res['time'] - grouped['time'].transform('min')).dt.total_seconds()
        // res['dt']).astype('int64')
    res.drop(columns=['time', 'dt'], inplace=True)
    return res


def print_trs(df: pandas.DataFrame, subdir: pathlib.Path) -> None:
    '''Export trajectories to files.'''
    # Ignore mode 99 (wait)
    for mode in [1, 2, 3, 4]:
        vprint(f'writing mode {mode}')
        cur_dir = subdir / str(mode)
        current = df.groupby('mode').get_group(mode)
        sp, dn, tot = split_count(current, cur_dir, True)
        vprint(f'Total: {tot}, sparse: {len(sp)}, dense: {len(dn)}')
        vprint(f'{(len(sp) + len(dn)) / tot:.2%} valid, '
               f'of which {len(dn) / (len(sp) + len(dn)):.2%} dense')
        test, train = dn, sp
        if conf.dense:
            random.shuffle(dn)
            spl = len(dn) // 6
            test, train = sorted(dn[:spl]), sorted(dn[spl:])
        vprint(f'using {len(train)} training and {len(test)} testing '
               'trajectories')
        with open(cur_dir / 'train.txt', 'w', encoding='utf-8') as trfile:
            print(*train, sep='\n', file=trfile)
        with open(cur_dir / 'test.txt', 'w', encoding='utf-8') as tefile:
            print(*test, sep='\n', file=tefile)


def process_data(subdir: pathlib.Path, fine: bool = False,
                 search: bool = False) -> None:
    '''Split data into subtrajectories and discretise them.'''
    deltat = ({1: 1, 2: 5, 3: 5, 4: 4, 99: 1} if fine else
              {1: 40, 2: 47, 3: 70, 4: 20, 99: 1})
    deltax = ({1: 2, 2: 102, 3: 130, 4: 21, 99: 1} if fine else
              {1: 80, 2: 960, 3: 1820, 4: 105, 99: 1})

    df = project_data(subdir)

    if search:
        goal_dense = 98 if conf.dense else 15
        deltat, deltax = finetune(df, subdir, goal_dense)
        vprint(f'dt: {deltat}\ndx: {deltax}')

    prep = discretise(df, deltat, deltax)
    print_trs(prep, subdir)


def main() -> None:
    '''Download and prepare the data.'''
    args = parse_args()
    if args.verbose:
        conf.verbose = True
    if args.straight:
        conf.straight = {*args.straight}
    if args.use_diagonals:
        conf.diagonal = True
    if args.keep_original:
        conf.fill_in = False
    if args.zoom_out:
        conf.dense = True

    if not args.only_process:
        get_unpack_data(args.subdir)
    if not args.only_download:
        process_data(args.subdir, args.fine, args.search)


if __name__ == '__main__':
    main()
