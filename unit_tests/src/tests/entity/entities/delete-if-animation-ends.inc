// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {
namespace Entities {

namespace DeleteIfAnimationEndsEntity {

    buildFunctionTable(BaseEntityFunctionTable, DeleteIfAnimationEndsEntity)

allocateTmpWord(parameterSum)

// IN: A = parameter
// OUT: C set = always activate entity
a16()
i16()
code()
function Init {
    lda.w   #MSEO.DeleteIfAnimationEnds.Animations.animation
    jsr     MetaSprite.SetAnimation

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
    jmp     Entity.DeleteIfAnimationEndsOrDrawAndGotoNextEntity
}


rodata(EN_RomData)
RomData:
    dw  FunctionTable
    db  0                            // initialListId
    db  0                            // defaultPalette
    dw  MSFS.DeleteIfAnimationEnds   // frameSetId

rodata(EN_EntityDataList)
constant EntityDataId = (pc() - Entity.Data.EntityDataList) / 2
    dw  RomData

code()
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

