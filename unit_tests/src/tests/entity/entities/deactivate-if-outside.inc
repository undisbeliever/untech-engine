// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Entities {

namespace DeactivateIfOutside {

    // Number of times the entity was activated and processed
    // (word counter)
    allocateTmpWord(activatedCount)

    // Number of times Deactivated is called
    // (word counter)
    allocateTmpWord(deactivatedCount)


    buildFunctionTable(BaseEntityFunctionTable, DeactivateIfOutside)

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(state, 2)
        endstruct()
    }

    namespace State {
        createEnum(0, 2)
            enum(DEACTIVATED)
            enum(ACTIVE)
        endEnum()

        constant MASK = 0x1 << 1

        code()
        FunctionTable:
            dw  Process_Deactiaved
            dw  Process_Active

            assert(pc() - FunctionTable == MASK + 2)
    }


// IN: A = parameter
// OUT: C set = always activate entity
a16()
i16()
code()
function Init {
    assert(State.DEACTIVATED == 0)
    stz.b   EntityStruct.state

    // only activate if inside active window
    clc
    rts
}


a16()
i16()
code()
Destructor:
HitboxCollision:
HurtboxCollision:
ShieldCollision:
function Null {
    rts
}

a16()
i16()
code()
function Deactivated {
    assert(State.DEACTIVATED == 0)
    stz.b   EntityStruct.state

    inc.w   deactivatedCount
    rts
}


a16()
i16()
code()
function Process {
    assertPowerOfTwo(State.MASK + 1 + 1)

    lda.b   EntityStruct.state
    and.w   #State.MASK
    tax
    jmp     (State.FunctionTable,x)
}


a16()
i16()
code()
function Process_Deactiaved {
    inc.w   activatedCount

    ldx.w   #State.ACTIVE
    stx.b   EntityStruct.state

    jmp     Entity.DeactivateIfOutsideAndGotoNextEntity
}


a16()
i16()
code()
function Process_Active {
    jmp     Entity.DeactivateIfOutsideAndGotoNextEntity
}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:


