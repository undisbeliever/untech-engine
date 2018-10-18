// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {
namespace Entities {

namespace ChangeToParticleEntity {

    buildFunctionTable(BaseEntityFunctionTable, ChangeToParticleEntity)
    Entity.useDefaultRenderer()

// IN: A = parameter
a16()
i16()
code()
function Init {
    rts
}


a16()
i16()
code()
Destructor:
ProcessCollision:
function Null {
    rts
}


a16()
i16()
code()
function Process {
    // change to particle list

    lda.w   #Entity.ListId.PARTICLE
    jmp     Entity.ChangeEntityListIdAndGotoNextEntity
}


evaluate n = 0
while {n} < Entity.lists.N_LISTS {
    rodata(EN_RomData)
    RomData{n}:
        dw  FunctionTable
        db  {n}             // initialListId
        db  0               // defaultPalette
        dw  0               // frameSetId

    rodata(EN_EntityList)
    constant EntityId_{n} = (pc() - Entity.Data.EntityList) / 2
        dw  RomData{n}

    evaluate n = {n} + 1
}

code()
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

