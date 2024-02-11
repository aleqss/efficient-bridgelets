#!/usr/bin/env python3

# Copyright 2023, 2024 Aleksandr Popov
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version. This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details. You should have received a copy of the GNU
# General Public License along with this program. If not, see
# <https://www.gnu.org/licenses/>.

import argparse
import cartopy.crs
import io
import multiprocessing
import pandas
import pathlib
import shutil
import sys
import urllib.request
import zipfile

def parse_args():
    '''Parse the arguments to the script, run with -h for help.'''
    parser = argparse.ArgumentParser(description='download and prepare '
        'OpenPFLOW data for the experiments', epilog='further links and '
        'explanation at https://github.com/aleqss/efficient-bridgelets')
    parser.add_argument('subdir', nargs='?', type=pathlib.Path,
        default=pathlib.Path(sys.argv[0]).resolve().parent,
        help='store the data in a given directory')
    acts = parser.add_mutually_exclusive_group()
    acts.add_argument('-d', '--only-download', action='store_true',
        default=False, help='skip processing the data')
    acts.add_argument('-p', '--only-process', action='store_true',
        default=False, help='process already available trajectories.tsv')
    parser.add_argument('-v', '--verbose', action='store_true',
        help='show progress information')
    args = parser.parse_args()
    if not args.subdir.is_dir():
        print('error: {} is not an existing directory'.format(args.subdir))
        parser.print_help()
        sys.exit(1)
    return args

def download(part: str):
    '''Download a partial archive and extract it.'''
    with urllib.request.urlopen(part) as sin:
        with zipfile.ZipFile(io.BytesIO(sin.read())) as archive:
            return archive.extract(archive.infolist()[0])

def get_unpack_data(subdir: pathlib.Path, verbose=False):
    '''Download the data, extract it, and concatenate the files in subdir.'''
    baseurl = ('https://open-research-data.s3.ap-northeast-1.amazonaws.com/'
               'openpflow/trajectory0')
    parts = (baseurl + str(i) + '.tsv.zip' for i in range(1, 10))

    if verbose:
        print('downloading files')
    with multiprocessing.Pool() as p:
        files = p.map(download, parts)

    if verbose:
        print('unpacking to the dataset file')
    with open(subdir / 'trajectories.tsv', 'wb') as comp:
        for file in files:
            with open(file, 'rb') as inp:
                shutil.copyfileobj(inp, comp)

    for file in files:
        pathlib.Path(file).unlink()

def process_data(subdir: pathlib.Path, verbose=False):
    '''Split data into subtrajectories and discretise them.'''
    deltax = {1: 120, 2: 510, 3: 2465, 4: 130, 99: 1}
    deltat = {1: 81, 2: 18, 3: 3, 4: 30, 99: 1}

    # Load the TSV file into a pandas dataframe
    if verbose:
        print('reading dataframe')
    df = pandas.read_csv(subdir / 'trajectories.tsv', sep='\t', header=None,
        names=['pid', 'time', 'lon', 'lat', 'mode'])
    df['time'] = pandas.to_datetime(df['time'], format="%Y-%m-%d %H:%M:%S")

    # Convert lon/lat to UTM zone 54 eastings/northings
    utmj = cartopy.crs.UTM(54)
    df[['easting', 'northing', 'z_dummy']] = \
        utmj.transform_points(cartopy.crs.PlateCarree(), df['lon'], df['lat'])
    df.drop(columns=['z_dummy', 'lon', 'lat'], inplace=True)

    # Discretise eastings/northings to x/y
    df['x'] = (df['easting'] // df['mode'].map(deltax)).astype('int64')
    df['y'] = (df['northing'] // df['mode'].map(deltax)).astype('int64')
    df.drop(columns=['easting', 'northing'], inplace=True)

    # Group by shifts in mode for each pid, compute time offset from start,
    # discretise the time steps
    df['dt'] = df['mode'].map(deltat)
    df['pnum'] = df['mode'].ne(df['mode'].shift()).cumsum()
    grouped = df.groupby(['pid', 'pnum'], sort=False)
    df['t'] = ((df['time'] - grouped['time'].transform('min')
               ).dt.total_seconds() // df['dt']).astype('int64')
    df.drop(columns=['time', 'dt'], inplace=True)

    # Avoid multiple measurements at the same time step
    df.drop_duplicates(subset=['mode', 'pid', 'pnum', 't'], inplace=True)

    # Unique entries per (mode, pid, pnum, t)
    # Ignore mode 99 (wait)
    ftemp = 'pid = {}, pnum = {}\n\n{}'
    for mode in {1, 2, 3, 4}:
        if verbose:
            print('writing mode {}'.format(mode))
        cur_dir = subdir / str(mode)
        cur_dir.mkdir(mode=0o755, exist_ok=True)
        current = df.groupby('mode').get_group(mode)
        fname = 0
        for (pid, pnum), gr in current.groupby(['pid', 'pnum'], sort=False):
            if len(gr) > 2:
                with open(cur_dir / str(fname), 'w') as cfile:
                    cfile.write(ftemp.format(pid, pnum, gr.to_csv(index=False,
                        columns = ['t', 'x', 'y'])))
                fname += 1
        if verbose:
            print('wrote {} files'.format(fname))

def main():
    '''Download and prepare the data.'''
    args = parse_args()
    if not args.only_process:
        get_unpack_data(args.subdir, args.verbose)
    if not args.only_download:
        process_data(args.subdir, args.verbose)

if __name__ == '__main__':
    main()
