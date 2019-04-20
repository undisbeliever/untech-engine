// errors/break.inc
// ================
//
// Macro to generate a brk interrupt with an identifying string.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


constant __BREAK__.first = 0x10

variable __BREAK__.pos = __BREAK__.first

// Register a break identifier
//
// If the `breakIdStrings` ROM Block exists, then a string with the
// value of `id` will be appended to the `breakIdStrings` ROM Block.
//
// This macro MUST BE called in the root scope.
//
// PARAM: id - warning identifier
inline RegisterBreakId(id) {
    if __BREAK__.pos > 255 {
        error "Too many Break IDs"
    }

    constant __BREAK__.ids.{id} = __BREAK__.pos
    __BREAK__.pos = __BREAK__.pos + 1

    if {defined __MEMORY__.romBlocks.breakIdStrings} {
        pushBlock()
        rodata(breakIdStrings)
            db  "{id}", 0
        popBlock()
    }
}

// Raise a BRK Interrupt.
//
// The break identifier must be registered with `RegisterBreakId`
//
// PARAM: id - break identifier
macro break(id) {
    brk     #__BREAK__.ids.{id}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

