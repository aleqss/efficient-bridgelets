#!/usr/bin/env python3

# Copyright 2022 Aleksandr Popov
# This program is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version. This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details. You should have received a copy of the GNU
# General Public License along with this program. If not, see
# <https://www.gnu.org/licenses/>.

import math
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import seaborn as sns
from pathlib import Path

picwidth = 3.0 # Could infer this from \textwidth if needed

def plot(filename, logscale):
    with open(filename, 'r') as infile:
        T = int(infile.readline())
    table = pd.read_table(filename, sep=' ', skiprows=1, names=range(-T, T + 1))
    nonp = False
    for column in table:
        if table[column].dtype == 'object':
            nonp = True
            table[column] = table[column].apply(int)
    table.index = range(-T, T + 1)
    table = table[table.columns[np.sum(table != 0) > 0]]
    table = table.loc[table.index[np.sum(table != 0, axis=1) > 0]]
    data = table
    print(filename)
    print(logscale)
    print(np.sum(np.sum(table != 0)))
    if nonp and logscale:
        data += 1
        for column in data:
            data[column] = data[column].apply(math.log)
    elif logscale:
        data = np.log(table + 1)
    scale = float(table.max().max() // (2 ** 30))
    print(scale)
    if scale > 0:
        data /= scale
    print(np.sum(np.sum(data != 0)))
    for column in data:
        data[column] = data[column].apply(float)
    print(np.sum(np.sum(data != 0)))
    print()
    mask = (data == 0)
    pallog = sns.cubehelix_palette(rot=1, gamma=.65, light=.9, dark=.05, hue=.9,
        as_cmap=True)
    palraw = 'flare'
    ax = sns.heatmap(data, xticklabels = 100, yticklabels=100,
        cmap=pallog if logscale else palraw, mask=mask, square=True)
    ax.invert_yaxis()
    plt.yticks(rotation=0)
    svdir = Path('.') / 'figs'
    fname_base = filename.name + '_' + ('log' if logscale else 'raw')
    # plt.savefig(svdir / (fname_base + '.pgf'))
    plt.savefig(svdir / (fname_base + '.png'), dpi=2500)
    plt.close()

def traj(cnt, si, sj, ei, ej):
    for i in range(cnt):
        filename = Path('.') / 'data' / ('traj' + str(i))
        if filename.is_file():
            t = np.loadtxt(filename, dtype=int)
            jitter = .1 * (i - cnt // 2)
            data = t + jitter
            plt.plot(data[:,0], data[:,1], lw=.8)
    plt.plot(si, sj, marker='o', markersize=5, color='black')
    plt.plot(ei, ej, marker='x', markersize=5, color='black')
    plt.yticks(range(sj, ej + 1, 10))
    plt.xticks(range(si, ei + 1, 10))
    svdir = Path('.') / 'figs'
    plt.savefig(svdir / 'trajectories.pgf')
    plt.savefig(svdir / 'trajectories.pdf')
    plt.close()

def main():
    data = Path('.') / 'data'
    fnames = ['paths_dp', 'visits_dp', 'wall', 'sm_wall', 'wall_gap',
        'sm_wall_gap']
    for filename in fnames:
        fullname = data / filename
        if fullname.is_file():
            for logscale in [True, False]:
                plt.figure(figsize=(picwidth, .85 * picwidth))
                plot(fullname, logscale)

    plt.figure(figsize=(2.0 * picwidth, picwidth))
    traj(5, 0, 0, 40, 20)

if __name__ == '__main__':
    main()
