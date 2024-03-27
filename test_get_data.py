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

'''
Simple testing on get_data.
'''

import pytest
import get_data as gd

diag_inst = [(0, 0, 3, 3, 3), (0, 0, 5, 3, 5), (-1, -1, 3, -3, 4),
    (1, 1, 1, 1, 0)]
rect_inst = [(0, 0, 3, 5, 8), (-1, -2, 3, 4, 10), (1, 1, 1, 1, 0)]
lv_inst = [([], [], []), ([], [0, 0, 0], [])]

@pytest.mark.parametrize('x1,y1,x2,y2,res', diag_inst)
def test_diag_length(x1, y1, x2, y2, res):
    assert gd.diag_length(x1, x2, y1, y2) == res

@pytest.mark.parametrize('x1,y1,x2,y2,res', rect_inst)
def test_rect_length(x1, y1, x2, y2, res):
    assert gd.rect_length(x1, x2, y1, y2) == res

@pytest.mark.parametrize('buf,last,res', lv_inst)
def test_last_valid_diag(buf, last, res):
    gd.conf.diagonal = True
    assert gd.last_valid(buf, last) == res

# def test_last_valid():
#     get_data.last_valid()
