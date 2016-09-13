
scope MetaSprite {
scope EntityHitbox {
scope CollisionTable {

allocateTmpWord(tmp)

constant TABLE_MASK(Format.EntityHitboxType._TABLE_MASK)

// Processes the collision
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = first entity
//         Y = second entity
// MODIFIES: DP
code()
scope FunctionTable: {
    macro _repeatNull(evaluate n) {
        if {n} > 0 {
            dw  Null
            _repeatNull({n} - 1)
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
        global define __first({name})
        _processList(_second)

        // padding
        assert({Format.EntityHitboxType.__enum.current} == 6 * 2)
        assert(Format.EntityHitboxType._TABLE_SHIFT == 3)
        _repeatNull(2)
    }
    _processList(_first)

    // more padding
    assert({Format.EntityHitboxType.__enum.current} == 6 * 2)
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
scope Null: {
    rts
}


a16()
i16()
code()
BodyWeak_BodyWeak:
BodyWeak_Body:
Body_BodyWeak:
scope Body_Body: {
    // A.CollideWithBody
    // B.CollideWithBody

    _simple(CollideWithBody, CollideWithBody)
}


a16()
i16()
code()
Body_BodyAttack:
BodyAttack_Attack:
scope Body_Attack: {
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
scope BodyWeak_Attack: {
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
scope Body_Shield: {
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
scope Body_ShieldAttack: {
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
scope BodyWeak_ShieldAttack: {
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
scope Shield_Body: {
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
scope Shield_Shield: {
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
scope Shield_Attack: {
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
scope ShieldAttack_Body: {
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
scope ShieldAttack_BodyWeak: {
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
scope ShieldAttack_BodyAttack: {
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
scope Attack_Body: {
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
scope Attack_BodyWeak: {
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
scope Attack_Shield: {
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
scope BodyAttack_BodyAttack: {
    // B.CalculateAttackValue (normal)
    // A.TakeDamageFromEntity (normal)

    // A.CalculateAttackValue (normal)
    // B.TakeDamageFromEntity (normal)

    phd
    sty.w   tmp

    // swap DP and Y
    tdc
    tax
    tya
    tcd
    txy

    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.CalculateAttackValue,x)


    pld
    ldy.w   tmp
    ldx.b   BaseEntity.functionTable
    clc
    jsr     (BaseEntityFunctionTable.TakeDamageFromEntity,x)


// DP = A
// Y = B
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
scope Attack_Attack: {
    // phase through each other
    rts
}

}
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

