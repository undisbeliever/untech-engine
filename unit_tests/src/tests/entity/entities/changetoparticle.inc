// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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


evaluate n = 0
while {n} < Entity.N_ENTITY_LISTS {
    rodata(EN_RomData)
    RomData{n}:
        dw  FunctionTable
        db  {n}             // initialListId
        db  0               // defaultPalette
        dw  0               // frameSetId

    rodata(EN_EntityDataList)
    constant EntityDataId_{n} = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData{n}

    evaluate n = {n} + 1
}

code()
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

