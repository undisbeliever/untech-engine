// metasprite/entityhitbox-collisions.inc
// ======================================
//
// MetaSprite Entity-Hitbox collision behaviour.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {
namespace EntityHitbox {
namespace CollisionTable {

allocateTmpWord(tmp)

constant TABLE_MASK = Format.EntityHitboxType._TABLE_MASK

// Processes the collision
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = first entity
//         Y = second entity
// MODIFIES: DP
a16()
i16()
code()
function FunctionTable {
    macro _repeatNull(evaluate n) {
        while {n} > 0 {
            dw  Null
            evaluate n = {n} - 1
        }
    }

    macro _processList(m) {
        // list order must match MetaSprite.Format.EntityHitboxType
        {m}(Body)
        {m}(BodyWeak)
        {m}(BodyAttack)
        {m}(Shield)
        {m}(ShieldAttack)
        {m}(Attack)
    }

    macro _second(name) {
        dw  {__first}_{name}
    }
    macro _first(name) {
        define __first = {name}
        _processList(_second)

        // padding
        assert({Format.EntityHitboxType.__ENUM__.current} == 6 * 2)
        assert(Format.EntityHitboxType._TABLE_SHIFT == 3)
        _repeatNull(2)
    }
    _processList(_first)

    // more padding
    assert({Format.EntityHitboxType.__ENUM__.current} == 6 * 2)
    _repeatNull(2 * 8)

    assert((pc() - FunctionTable) == TABLE_MASK + 2)
}


macro _simple(a, b) {
    phy

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.{a},x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.{b},x)
}

a16()
i16()
code()
function Null {
    rts
}


a16()
i16()
code()
BodyWeak_BodyWeak:
BodyWeak_Body:
Body_BodyWeak:
function Body_Body {
    // A.CollideWithBody
    // B.CollideWithBody

    _simple(CollideWithBody, CollideWithBody)
}


a16()
i16()
code()
Body_BodyAttack:
BodyAttack_Attack:
function Body_Attack {
    // B.CalculateAttackValue (normal)
    // A.TakeDamageFromEntity (normal)

    phd
    phy

    // swap DP and Y
    tdc
    tax
    tya
    tcd
    txy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

// restore A
    ply
    pld
    ldx.b   BaseEntity.functionTable
    clc
    jmp     (BaseEntityFunctionTable.TakeDamageFromEntity,x)
}


a16()
i16()
BodyWeak_BodyAttack:
function BodyWeak_Attack {
    // B.CalculateAttackValue (normal)
    // A.TakeDamageFromEntity (weak)

    phd
    phy

    // swap DP and Y
    tdc
    tax
    tya
    tcd
    txy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

// restore A
    ply
    pld
    ldx.b   BaseEntity.functionTable
    sec
    jmp     (BaseEntityFunctionTable.TakeDamageFromEntity,x)
}


a16()
i16()
code()
BodyWeak_Shield:
function Body_Shield {
    // A.CollideWithShield
    // B.BlockWithShield (body)

    phy

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.CollideWithShield,x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    clc
    jmp     (BaseEntityFunctionTable.BlockWithShield,x)
}


a16()
i16()
code()
BodyAttack_ShieldAttack:
function Body_ShieldAttack {
    // B.CalculateAttackValue (shield)
    // A.TakeDamageFromEntity (normal)
    // A.CollideWithShield

    phd
    sty.w   tmp

    // swap DP and Y
    tdc
    tax
    tya
    tcd
    txy

    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)


    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)


    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
function BodyWeak_ShieldAttack {
    // B.CalculateAttackValue (shield)
    // A.TakeDamageFromEntity (weak)
    // A.CollideWithShield

    phd
    sty.w   tmp

    // swap DP and Y
    tdc
    tax
    tya
    tcd
    txy

    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)


    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)


    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
Shield_BodyWeak:
function Shield_Body {
    // A.BlockWithShield (body)
    // B.CollideWithShield

    phy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.BlockWithShield,x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
ShieldAttack_ShieldAttack:
ShieldAttack_Shield:
Shield_ShieldAttack:
function Shield_Shield {
    // A.BlockWithShield (body)
    // B.BlockWithShield (body)

    phy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.BlockWithShield,x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    clc
    jmp     (BaseEntityFunctionTable.BlockWithShield,x)
}


a16()
i16()
code()
Shield_BodyAttack:
ShieldAttack_Attack:
function Shield_Attack {
    // A.BlockWithShield (attack)
    // B.CollideWithShield

    phy

    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.BlockWithShield,x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
function ShieldAttack_Body {
    // A.CalculateAttackValue (shield)
    // B.TakeDamageFromEntity (normal)
    // B.CollideWithShield

    tdc
    sta.w   tmp
    phy

    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)

    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
function ShieldAttack_BodyWeak {
    // A.CalculateAttackValue (shield)
    // B.TakeDamageFromEntity (weak)
    // B.CollideWithShield

    tdc
    sta.w   tmp
    phy

    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)

    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
function ShieldAttack_BodyAttack {
    // A.CalculateAttackValue (shield)
    // B.TakeDamageFromEntity (normal)
    // B.CollideWithShield

    tdc
    sta.w   tmp
    phy

    lda.w   tmp
    ldx.b   BaseEntity.functionTable
    sec
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)

    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    jmp     (BaseEntityFunctionTable.CollideWithShield,x)
}


a16()
i16()
code()
BodyAttack_Body:
Attack_BodyAttack:
function Attack_Body {
    // A.CalculateAttackValue (normal)
    // B.TakeDamageFromEntity (normal)

    phd
    phy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

// swap to B
    pld
    ply
    ldx.b   BaseEntity.functionTable
    clc
    jmp     (BaseEntityFunctionTable.TakeDamageFromEntity,x)
}


a16()
i16()
code()
BodyAttack_BodyWeak:
function Attack_BodyWeak {
    // A.CalculateAttackValue (attack)
    // B.TakeDamageFromEntity (weak)

    phd
    phy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)

// swap to B
    pld
    ply
    ldx.b   BaseEntity.functionTable
    sec
    jmp     (BaseEntityFunctionTable.TakeDamageFromEntity,x)
}


a16()
i16()
code()
BodyAttack_Shield:
Attack_ShieldAttack:
function Attack_Shield {
    // A.CollideWithShield
    // B.BlockWithShield (attack)

    phy

    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.CollideWithShield,x)

    tdc
    tay
    pld

    ldx.b   BaseEntity.functionTable
    sec
    jmp     (BaseEntityFunctionTable.BlockWithShield,x)
}


a16()
i16()
code()
function BodyAttack_BodyAttack {
    // A.CalculateAttackValue (normal)
    // B.TakeDamageFromEntity (normal)
    // B.CalculateAttackValue (normal)
    // A.TakeDamageFromEntity (normal)

    tdc
    sta.w   tmp
    phy

// DP = A
// Y = B
    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)


// DP = B
// Y = A
    pld
    ldy.w   tmp

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)


// DP = B
// Y = A
    phd
    ldy.w   tmp

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)


// DP = B
// Y = A
    lda.w   tmp
    tcd

    ply
    ldx.b   BaseEntity.functionTable
    clc
    jmp     (BaseEntityFunctionTable.TakeDamageFromEntity,x)
}


a16()
i16()
code()
function Attack_Attack {
    // phase through each other
    rts
}

}
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

