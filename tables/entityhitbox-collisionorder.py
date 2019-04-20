#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:

"""
Entity Hithox collision order table generator.
"""

# This file is part of the UnTech Game Engine.
# Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
# Distributed under The MIT License: https://opensource.org/licenses/MIT


MAX_INNER_HITBOXES = 4
LIST_SHIFT = 2

INNER_OFFSET = 5

def main():
    pos = 0
    matches = dict()

    def printAddress(a, b):
        nonlocal pos
        aOffset = INNER_OFFSET * a
        bOffset = INNER_OFFSET * b
        print("\tdb ", aOffset, ", ", bOffset, sep="")
        pos += 2

    def printNull():
        nonlocal pos
        print("\tdw 0xffff")
        pos += 2


    print("namespace Tables {")
    print("namespace EntityHitbox {")
    print("constant INNER_OFFSET =", INNER_OFFSET)
    print("constant MAX_INNER_HITBOXES =", MAX_INNER_HITBOXES)

    print("rodata(rom0)")
    print("CollisionOrder:")

    # count == 1 is processed by other code

    for toTest in range(1, MAX_INNER_HITBOXES):
        for current in range(1, MAX_INNER_HITBOXES):
            matches[(toTest, current)] = pos

            print("//", toTest + 1, current + 1)

            collisions = list()
            for i in range(toTest + 1):
                for j in range(current + 1):
                    collisions.append((i, j))

            collisions.sort(key=lambda x : min(x) * 10 + max(x))

            for c in collisions:
                printAddress(c[0], c[1])
            printNull()

    print()
    print("CollisionOrderTable:")
    print("constant CollisionOrderTable.SHIFT =", LIST_SHIFT)
    print("// NOTE: this table ignores toTest EntityHitbox.count == 0")
    print("// table index = (((toTest.count - 1) << LIST_SHIFT) | current.count) * 2")

    m = 1 << LIST_SHIFT
    assert(MAX_INNER_HITBOXES <= m)
    for toTest in range(1, m):
        l = list()
        for current in range(0, m):
            t = (toTest, current)
            l.append(str(matches.get(t, 0)))

        print("\tdw ", ", ".join(l))

    print()
    print()
    print("code()")
    print("}")
    print("}")


if __name__ == "__main__":
    main()

