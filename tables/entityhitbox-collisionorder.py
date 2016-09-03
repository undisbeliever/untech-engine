#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:

MAX_INNER_HITBOXES = 4
LIST_SHIFT = 2

def main():
    pos = 0
    matches = dict()

    def printAddress(a, b):
        nonlocal pos
        aOffset = 7 * a
        bOffset = 7 * b
        print("\tdb ", aOffset, ", ", bOffset, sep="")
        pos += 2

    def printNull():
        nonlocal pos
        print("\tdw 0xffff")
        pos += 2


    print("scope Tables {")
    print("scope EntityHitbox {")
    print("constant MAX_INNER_HITBOXES (", MAX_INNER_HITBOXES, ")", sep="")

    print("rodata(rom0)")
    print("scope CollisionOrder: {")

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

    print("}")

    print("scope CollisionOrderTable: {")
    print("constant SHIFT(", LIST_SHIFT, ")", sep="")
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

    print("}")
    print("\ncode()")
    print("}")
    print("}")


if __name__ == "__main__":
    main()

