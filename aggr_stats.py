#!/usr/bin/env -S python3 -i

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

import pathlib
import pandas

df: dict[int, pandas.DataFrame] = {}
pandas.options.display.max_columns = None

for pr in ['-lin', '-uni']:
    for i in range(1, 5):
        fname = 'm' + str(i) + pr + '-diag-t0-0.1-0.5-none-sp'
        floc = pathlib.Path('analysis') / str(i) / fname

        df[i] = pandas.read_table(floc, sep=' ', index_col='id')
        df[i]['cov'] = df[i]['cov'].astype(bool)

        print('mode', str(i), pr)
        print('full')
        print(df[i].describe())
        print('covered')
        print(df[i][df[i]['cov']].describe())
