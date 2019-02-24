// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Entities {

namespace Blank {

    buildFunctionTable(BaseEntityFunctionTable, Blank)

allocateTmpWord(parameterSum)

// IN: A = parameter
// OUT: C set = always activate entity
a16()
i16()
code()
function Init {
    clc
    adc.w   parameterSum
    sta.w   parameterSum

    // always activate
    sec
    rts
}


a16()
i16()
code()
Destructor:
Deactivated:
ProcessCollision:
function Null {
    rts
}


a16()
i16()
code()
function Process {
    jmp     Entity.GotoNextEntity
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

