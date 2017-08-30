// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace EntityTest {
namespace Entities {

namespace DeleteAfterDelayEntity {

    buildFunctionTable(BaseEntityFunctionTable, DeleteAfterDelayEntity)
    Entity.useDefaultRenderer()

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(frames, 2)
        endstruct()
    }

// IN: A = parameter
a16()
i16()
code()
function Init {
    sta.b   EntityStruct.frames
    rts
}


a16()
i16()
code()
function Destructor {
    rts
}


a16()
i16()
code()
function Process {
    dec.b   EntityStruct.frames
    bne     +
        lda.w   #Entity.lists.toDelete
        sta.w   Entity.changeList
    +

    rts
}


a16()
i16()
code()
CalculateAttackValue:
TakeDamageFromEntity:
BlockWithShield:
CollideWithBody:
CollideWithShield:
    rts



evaluate n = 0
while {n} < Entity.lists.N_LISTS {
    rodata(EN_RomData)
    RomData{n}:
        dw  FunctionTable
        db  {n} * 2         // initial List
        dw  0               // frameSetId
        db  0               // defaultPalette

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

