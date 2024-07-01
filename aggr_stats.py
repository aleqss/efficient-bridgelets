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

'''
Import the test results and summarise them.
'''

import pathlib
import pandas

def main() -> list[dict[int, pandas.DataFrame]]:
    '''Process the test results.'''
    df: list[dict[int, pandas.DataFrame]] = [{}, {}]
    pandas.options.display.max_columns = None # type: ignore

    for ind, name in enumerate(['-lin', '-uni']):
        for i in range(1, 5):
            fname = 'm' + str(i) + name + '-diag-t0-0.1-0.4-0.5-0.6-none-sp'
            floc = pathlib.Path('analysis') / str(i) / fname

            df[ind][i] = pandas.read_table(floc, sep=' ', index_col='id')
            df[ind][i]['cov'] = df[ind][i]['cov'].astype(bool)

            print('mode', str(i), 'linger' if ind == 0 else 'uniform')
            print('full')
            print(df[ind][i].describe())
            print('covered')
            print(df[ind][i][df[ind][i]['cov']].describe())
    return df

if __name__ == '__main__':
    data = main()
