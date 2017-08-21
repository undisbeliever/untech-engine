// errors/break.inc
// ================
//
// Macro to generate a brk interrupt with an identifying string.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


evaluate ::breakId.pos(0)

// Raise a BRK Interrupt.
//
// PARAM: id - break identifier
//
// Identifiers can be repeated, duplicate identifiers will resolve to
// the same BRK id.
//
// If the `breakIdStrings` ROM Block exists, then a string with the
// value of `id` will be appended to the `breakIdStrings` ROM Block.
macro break(id) {
    if !{defined ::breakId.ids.{id}} {
        if {::breakId.pos} > 255 {
            error "Too many Break IDs"
        }

        global define ::breakId.ids.{id}({::breakId.pos})
        global evaluate ::breakId.pos({::breakId.pos} + 1)

        if {defined __MEMORY__.romBlocks.breakIdStrings} {
            pushBlock()
            rodata(breakIdStrings)
                db  "{id}", 0
            popBlock()
        }
    }

    brk     #{::breakId.ids.{id}}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

