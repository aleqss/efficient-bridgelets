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

import subprocess
from pathlib import Path

base_1 = '\\documentclass[multi]{standalone}\n' + \
    '\\usepackage{amsmath,amssymb,import,newpxmath,newpxtext,pgf}\n' + \
    '\\standaloneenv{pgfpicture}\n\n\\begin{document}%\n\\import{figs}{'
base_2 = '.pgf}\n\\end{document}\n'

fig_names = ['paths_dp_raw', 'paths_dp_log', 'visits_dp_raw', 'visits_dp_log',
    'wall_gap_raw', 'wall_gap_log', 'sm_wall_gap_raw', 'sm_wall_gap_log',
    'sm_wall_raw', 'sm_wall_log', 'trajectories']

for fname in fig_names:
    with open(fname + '.tex', 'w') as fout:
        fout.write(base_1 + fname + base_2)
    subprocess.call(['latexmk', '-pdf', fname + '.tex'])
    subprocess.call(['inkscape', '-l', '--export-filename=' + fname + '.svg',
        fname + '.pdf'])

subprocess.call(['latexmk', '-pdf', '-C'])

for fname in fig_names:
    Path(fname + '.tex').unlink(missing_ok=True)
