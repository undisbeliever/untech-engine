// entity/invalid-entity.inc
// =========================
//
// Function table for invalid (deleted) entities.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ENTITY_FUNCTION)


namespace Entity {

// This function table contains 12 entries to `InvalidEntityFunction`
//
// It is used to notify the user when the function table of a deleted
// entity is accidentally called.
code()
InvalidEntityFunctionTable:
namespace InvalidEntityFunctionTable {
    // contains 20 as BaseEntityFunctionTable can be extended
    evaluate n = 20
    assert({n} > BaseEntityFunctionTable.size)

    while {n} > 0 {
        dw      InvalidEntityFunction
        evaluate n = {n} - 1
    }
}


// Breaks execution with INVALID_ENTITY_FUNCTION.
au()
iu()
code()
function InvalidEntityFunction {
    break(INVALID_ENTITY_FUNCTION)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

