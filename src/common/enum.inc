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
//
//        enum(initPtr)    // 0
//        enum(deletePtr)  // 2
//        enum(processPtr) // 4
//    }
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


inline createEnum(start, increment) {
    if {increment} < 1 {
        error "increment is invalid"
    }

    constant __ENUM__.first = {start}
    evaluate __ENUM__.current = {start}
    evaluate __ENUM__.increment = {increment}
}
inline createEnum(start) {
    createEnum({start}, 1)
}
inline createEnum() {
    createEnum(0, 1)
}

inline enum(id) {
    if {defined __ENUM__.current} {
        constant {id} = {__ENUM__.current}
        evaluate __ENUM__.current = {__ENUM__.current} + {__ENUM__.increment}

    } else {
        error "Current scope is not an enum"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

