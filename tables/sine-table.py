#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:

"""
8 bit Sine Table generator
"""

# This file is part of the UnTech Game Engine.
# Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
# Distributed under The MIT License: https://opensource.org/licenses/MIT

import math

N_ROTATIONS = 256
DW_PER_LINE = 8

def main():
    print("constant SineTable.size =", N_ROTATIONS)
    print("")
    print("rodata(rom0)")
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

