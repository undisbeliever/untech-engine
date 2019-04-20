// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Entities {

namespace ChangeToParticle {

    buildFunctionTable(BaseEntityFunctionTable, ChangeToParticle)

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
    // change to particle list

    lda.w   #EntityLists.Particles
    jmp     Entity.ChangeEntityListIdAndGotoNextEntity
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

