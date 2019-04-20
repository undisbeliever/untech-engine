// common/enum.inc
// ===============
//
// Turns the current scope into an enumeration.
//
// Enum constants are declared with the enum macro.
//
// NOTE: An enum cannot be nested in a scope
//
// usage:
//    namespace functionTable {
//        createEnum(0, 2)
//          enum(initPtr)    // 0
//          enum(deletePtr)  // 2
//          enum(processPtr) // 4
//        endEnum()          // (optional but highly recommended)
//    }
//
// The `createEnum()`/`endEnum()` pair will create the following constants:
//
//  * `__ENUM__.first`: the first value in the enum
//  * `__ENUM__.last`: the last value in the enum
//  * `__ENUM__.increment`: the difference between two successive enum values
//  * `__ENUM__.count`: the number of items in the enum
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


inline createEnum(start, increment) {
    if {increment} < 1 {
        error "increment is invalid"
    }

    constant __ENUM__.first = {start}
    constant __ENUM__.increment = {increment}
    evaluate __ENUM__.current = {start}
}
inline createEnum(start) {
    createEnum({start}, 1)
}
inline createEnum() {
    createEnum(0, 1)
}

// NOTE: This tag is optional but highly recommended
inline endEnum() {
    if !{defined __ENUM__.current} {
        error "Current scope is not an enum"
    }
    if {defined __ENUM__.closed} {
        error "Enum already closed"
    }
    if {__ENUM__.current} == __ENUM__.first {
        error "Expected at least one enum()"
    }
    evaluate __ENUM__.closed = 1
    constant __ENUM__.last = {__ENUM__.current} - __ENUM__.increment
    constant __ENUM__.count = ({__ENUM__.current} - __ENUM__.first) / __ENUM__.increment
}

inline enum(id) {
    if {defined __ENUM__.closed} {
        error "Enum is closed, cannot add more values"
    }
    if !{defined __ENUM__.current} {
        error "Current scope is not an enum"
    }

    constant {id} = {__ENUM__.current}
    evaluate __ENUM__.current = {__ENUM__.current} + __ENUM__.increment
}

// vim: ft=bass-65816 ts=4 sw=4 et:

