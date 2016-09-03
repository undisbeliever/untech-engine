#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# vim: set fenc=utf-8 ai ts=4 sw=4 sts=4 et:

MAX_HITBOXES = 6
LIST_SHIFT = 3

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
    print("rodata(rom0)")
    print("scope CollisionOrder: {")

    for toTest in range(1, MAX_HITBOXES):
        for current in range(1, MAX_HITBOXES):
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

    print("scope CollisionOrderList: {")
    print("constant SHIFT(", LIST_SHIFT, ")", sep="")
    print("// this lists ignores toTest.nhitboxes == 1")

    m = 1 << LIST_SHIFT
    assert(MAX_HITBOXES <= m)
    for toTest in range(1, m):
        for current in range(0, m):
            t = (min(toTest, MAX_HITBOXES - 1), min(current, MAX_HITBOXES - 1))
            print("\tdw", matches.get(t, 0))

    print("}")
    print("\ncode()")
    print("}")
    print("}")


if __name__ == "__main__":
    main()

