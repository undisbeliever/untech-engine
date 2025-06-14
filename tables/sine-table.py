#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:

"""
8 bit Sine Table generator
"""

# SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
# SPDX-License-Identifier: Zlib
#
# Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
#
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from the
# use of this software.
#
# Permission is granted to anyone to use this software for any purpose, including
# commercial applications, and to alter it and redistribute it freely, subject to
# the following restrictions:
#
#    1. The origin of this software must not be misrepresented; you must not
#       claim that you wrote the original software. If you use this software in
#       a product, an acknowledgment in the product documentation would be
#       appreciated but is not required.
#
#    2. Altered source versions must be plainly marked as such, and must not be
#       misrepresented as being the original software.
#
#    3. This notice may not be removed or altered from any source distribution.


import math

N_ROTATIONS = 256
DW_PER_LINE = 8

def main():
    print("constant SineTable.size =", N_ROTATIONS)
    print("")
    print("rodata()")
    print("SineTable:", end="")

    for a in range(N_ROTATIONS):
        if a % DW_PER_LINE == 0:
            print("\n  db ", end="")
        else:
            print(", ", end="")

        r = math.pi * 2 * a / 256
        s = int(math.sin(r) * 0x7f)
        if s < 0:
            s += 0x100

        print("{:#04x}".format(s), end="")

    print("\n")

if __name__ == "__main__":
    main()

