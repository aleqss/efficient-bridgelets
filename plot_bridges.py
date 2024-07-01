#!/usr/bin/env python3

# Copyright 2024 Aleksandr Popov
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
Plot the bridges and the original discretised trajectory.

This assumes that you ran the main program and requested for data for specific
trajectories, and now want to plot the resulting bridges.
'''

import argparse
import pathlib
import sys
from dataclasses import dataclass
from fractions import Fraction
from typing import Any

from matplotlib import colors, pyplot #type:ignore
import pandas

@dataclass(eq=False, kw_only=True)
class conf: # pylint: disable=C0103
    '''Global config.'''
    verbose: bool = False
    picwidth: float = 8.0

def vprint(*args: Any, **kwargs: Any) -> Any:
    '''A simple logging wrapper.'''
    if conf.verbose:
        print(*args, **kwargs)

def parse_args() -> argparse.Namespace:
    '''Parse the arguments to the script, run with -h for help.'''
    parser = argparse.ArgumentParser(description='plot the computed bridges, '
        'baseline beads, and trajectories for specified IDs', epilog='further '
        'information at https://github.com/aleqss/efficient-bridgelets')
    parser.add_argument('mode', type=int,
        help='the movement mode for the query')
    parser.add_argument('tr_id', nargs='+', type=int,
        help='one or more trajectory IDs')
    parser.add_argument('-t', '--trajectories', type=pathlib.Path,
        default=pathlib.Path(sys.argv[0]).resolve().parent / 'movement',
        help='directory containing discrete trajectory files')
    parser.add_argument('-b', '--bridges', type=pathlib.Path,
        default=pathlib.Path(sys.argv[0]).resolve().parent / 'analysis',
        help='directory containing computed bridge files')
    parser.add_argument('-o', '--output', type=pathlib.Path,
        default=pathlib.Path(sys.argv[0]).resolve().parent / 'figs',
        help='directory where figures will be generated')
    parser.add_argument('-v', '--verbose', action='store_true',
        help='show progress information')
    args = parser.parse_args()
    return args

def read_bridge(fname: pathlib.Path) -> pandas.DataFrame:
    '''
    Read the bridge.

    Input format: {x y} frac, one per line.
    Output format: dataframe with frac at loc[x, y], or 0 if not present.
    '''
    data = {}
    with open(fname, 'r', encoding='utf-8') as fin:
        for line in fin:
            pi, jp, frstr = line.split()
            i, j, fr = int(pi[1:]), int(jp[:-1]), Fraction(frstr)
            data[(i, j)] = fr

    ser = pandas.Series(list(data.values()),
        index=pandas.MultiIndex.from_tuples(data.keys()))
    df = ser.unstack().T.fillna(0)
    return df

def read_traj(fname: pathlib.Path) -> pandas.DataFrame:
    '''Read a discretised trajectory.'''
    return pandas.read_csv(fname, skiprows=2, index_col='t')

def extend_sh(list_like: pandas.Index) -> list[int]:
    '''Extend the list with the next element, shift by -0.5.'''
    ret = list(list_like)
    ret.append(ret[-1] + 1)
    return [x - 0.5 for x in ret]

def plot_bridge(fname: pathlib.Path, fname_tr: pathlib.Path,
        oname: pathlib.Path) -> None:
    '''Plot a bridge with the original trajectory.'''
    df = read_bridge(fname)
    tr = read_traj(fname_tr)
    pldf = df[df != 0].astype(float)

    fig, ax = pyplot.subplots(figsize=(conf.picwidth, .85 * conf.picwidth))
    norm = colors.LogNorm(vmin=pldf.stack().min(), vmax=pldf.stack().max())
    ax.set_aspect('equal')
    hm = ax.pcolormesh(extend_sh(df.columns), extend_sh(df.index), pldf,
        cmap='magma_r', norm=norm)
    ax.scatter(x=tr['x'], y=tr['y'])
    cb = fig.colorbar(hm)
    for obj in [ax.xaxis, ax.yaxis]:
        obj.get_major_locator().set_params(integer=True)
        obj.get_major_formatter().set_useOffset(False)
        obj.get_major_formatter().set_scientific(False)
    pyplot.savefig(oname)
    pyplot.close()

def main() -> None:
    '''Plot the requested bridges.'''
    args = parse_args()
    if args.verbose:
        conf.verbose = True
    trfiles = args.trajectories / str(args.mode)
    anfiles = args.bridges
    ofiles = args.output

    beads = ['main', 'mn_discr', 'mn_clamp', 'ellipse', 'br_discr', 'br_clamp',
        'straight']
    for trid in args.tr_id:
        vprint(f'processing trajectory {trid}:', end='', flush=True)
        for bd in beads:
            vprint(f' {bd}', end='', flush=True)
            plot_bridge(anfiles / str(args.mode) / str(trid) / bd,
                trfiles / str(trid), ofiles / (str(trid) + '_' + bd + '.png'))
        vprint()

if __name__ == '__main__':
    main()
