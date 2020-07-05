// metatiles/_mapheight.inc
// ==========================
//
// Macros for branching/jumping depending on map height.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaTiles {

// Confirm bit 7 of `map.bytesPerColumn` can be used to determine map height
assert(MAP_HEIGHT_SMALL & 0x80 == 0)
assert(MAP_HEIGHT_LARGE & 0x80 == 0x80)


// Branch to {label} if map height is small
//
// This macro is branch if small (not branch if large) as the large map height
// code uses more CPU time then the small height code.  Branching if small
// helps minimise the difference in CPU time for each code path.
//
// REQUIRES: DB = 0x7e
// PARAM: label - label to branch to if map height is small
// KEEP: A, X, Y
// BRANCH: {label} if map height is small
inline BranchIfMapIsSmall(label) {
    if __CPU__.aSize == 8 {
        bit.w   map.bytesPerColumn
        bpl     {label}

    } else if __CPU__.aSize == 16 {
        bit.w   map.bytesPerColumn - 1
        bpl     {label}

    } else {
        error "Unknown A size"
    }
}

// Branch to {label} if map height is small
//
// This macro is used if invoking `BranchIfMapIsSmall` causes a
// "branch out of bounds" assembly error.
//
// REQUIRES: DB = 0x7e
// PARAM: label - label to branch to if map height is small
// KEEP: A, X, Y
// BRANCH: {label} if map height is small
inline BranchIfMapIsSmall_Long(label) {
    if __CPU__.aSize == 8 {
        bit.w   map.bytesPerColumn
        bmi     _Large_{#}
            jmp     {label}
    _Large_{#}:

    } else if __CPU__.aSize == 16 {
        bit.w   map.bytesPerColumn - 1
        bmi     _Large_{#}
            jmp     {label}
    _Large_{#}:

    } else {
        error "Unknown A size"
    }
}

}

