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
//    scope functionTable {
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


macro createEnum(start, increment) {
    if {increment} < 1 {
        error "increment is invalid"
    }

    constant __enum.first({start})
    global evaluate __enum.current({start})
    global evaluate __enum.increment({increment})
}
macro createEnum(start) {
    createEnum({start}, 1)
}
macro createEnum() {
    createEnum(0, 1)
}

macro enum(id) {
    if {defined __enum.current} {
        constant {id}({__enum.current})
        global evaluate __enum.current({__enum.current} + {__enum.increment})

    } else {
        error "Current scope is not an enum"
    }
}

// vim: ft=bass-65816 ts=4 sw=4 et:

