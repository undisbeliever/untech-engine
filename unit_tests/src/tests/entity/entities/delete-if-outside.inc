// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Entities {

namespace DeleteIfOutside {

    buildFunctionTable(BaseEntityFunctionTable, DeleteIfOutside)

// IN: A = parameter
// OUT: C set = always activate entity
a16()
i16()
code()
function Init {
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
    jmp     Entity.DeleteIfOutsideAndGotoNextEntity
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:
