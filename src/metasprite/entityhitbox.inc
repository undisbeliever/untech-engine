// metasprite/entityhitbox.inc
// ===========================
//
// MetaSprite Entity-Hitbox collision tests.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {
namespace EntityHitbox {

// Temporary store for the offset/size pair from EntityHitbox struct
// Used to convert width/height from uint8 to uint16
// (3x uint8_t, third byte always 0)
allocate(_offsetSizePair, wram7e, 3)

// The current entity that is being tested
namespace entityA {
    // address of the entity
    // (word addr)
    allocateTmpWord(entityAddress)

    // address of the entity hitbox
    allocateTmpWord(hitboxAddress)

    // number of hitboxes
    // (only set in when entity has Multiple hitboxes)
    allocateTmpWord(hitboxCount)

    // Outer hitbox of entity
    // X/Y values are offset by INT_MS8_OFFSET
    namespace outer {
        allocateTmpWord(xPos)
        allocateTmpWord(yPos)
        allocateTmpWord(width)
        allocateTmpWord(height)
    }
}

// The entity in the list that is being tested
namespace entityB {
    allocateTmpWord(entityAddress)
    allocateTmpWord(hitboxAddress)

    // Used for both inner and outer hitbox tests
    namespace aabb {
        allocateTmpWord(xPos)
        allocateTmpWord(yPos)
        allocateTmpWord(width)
        allocateTmpWord(height)
    }
}



// Preforms a collision test between the current entity and an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity to test
// PARAM: list = the list to test with
macro ProcessList(list) {
    assert16a()
    assert16i()

    ldy.w   Entity.lists.{list}
    beq     Skip_{list}
        jsr     MetaSprite.EntityHitbox._TestList
Skip_{list}:
}


// Calculates the outer hitbox (enityhitbox address is known)
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
//         X = EntityHitbox address (NOT NULL)
// KEEP: X, Y
// BRANCH: to NoHitbox is there is no metasprite entityhitbox
macro _SetupOuterAabb(aabb) {
    assert16a()
    assert16i()

    lda.l   Format.EntityHitbox.Outer.xOffset,x
    and.w   #0xff
    clc
    adc.b   BaseEntity.xPos.px
    sta.w   {aabb}.xPos

    lda.l   Format.EntityHitbox.Outer.width,x
    and.w   #0xff
    sta.w   {aabb}.width


    lda.l   Format.EntityHitbox.Outer.yOffset,x
    and.w   #0xff
    clc
    adc.b   BaseEntity.yPos.px
    sta.w   {aabb}.yPos

    lda.l   Format.EntityHitbox.Outer.height,x
    and.w   #0xff
    sta.w   {aabb}.height
}


// Branches to label if there is no collision between the dp entity's
// outer hitbox and aabb
//
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity
// INPUT: X = EntityHitbox address
// KEEP: X, Y
// BRANCH: to `noCollision` if there is no collision
macro _BranchIfNoOuterCollision(aabb, NoCollision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        lda.l   Format.EntityHitbox.Outer.xOffset,x
        sta.w   _offsetSizePair
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px

        cmp.w   {aabb}.xPos
        bcs     XAhead
            // c clear
            adc.w   _offsetSizePair + 1     // width
            cmp.w   {aabb}.xPos
            bcc     {NoCollision}
            bra     XMatch
        XAhead:
            // c set
            sbc.w   {aabb}.width
            cmp.w   {aabb}.xPos
            bcs     {NoCollision}
        XMatch:


        lda.l   Format.EntityHitbox.Outer.yOffset,x
        sta.w   _offsetSizePair
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px
        cmp.w   {aabb}.yPos
        bcs     YAhead
            // c clear
            adc.w   _offsetSizePair + 1     // height
            cmp.w   {aabb}.yPos
            bcc     {NoCollision}
            bra     YMatch

        YAhead:
            // c set
            sbc.w   {aabb}.height
            cmp.w   {aabb}.yPos
            bcs     {NoCollision}
        YMatch:
    }
}


// Branches to `Collision` label if there is a collision
// between selected inner hitbox and given aabb.
//
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity to test
// INPUT: X = EntityHitbox address + inner collision offset
// KEEP: X, Y
// BRANCH: to `Collision` if there is a collision
macro _BranchOnInnerCollision(aabb, Collision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        lda.l   Format.EntityHitbox.Inner.xOffset,x
        sta.w   _offsetSizePair
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px

        cmp.w   {aabb}.xPos
        bcs     XAhead
            // c clear
            adc.w   _offsetSizePair + 1     // width
            cmp.w   {aabb}.xPos
            bcc     NoCollision
            bra     XMatch
        XAhead:
            // c set
            sbc.w   {aabb}.width
            cmp.w   {aabb}.xPos
            bcs     NoCollision
        XMatch:


        lda.l   Format.EntityHitbox.Inner.yOffset,x
        sta.w   _offsetSizePair
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px
        cmp.w   {aabb}.yPos
        bcs     YAhead
            // c clear
            adc.w   _offsetSizePair + 1     // height
            cmp.w   {aabb}.yPos
            bcs     {Collision}
            bra     NoCollision

        YAhead:
            // c set
            sbc.w   {aabb}.height
            cmp.w   {aabb}.yPos
            bcc     {Collision}

        NoCollision:
    }
}


// Process the collision
//
// Calls BaseEntityFunctionTable.ProcessCollision for both entities
// with the appropriate collisionBits
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: DP = this entity address
//         X = this inner collision address
//         ns = namespace holding the other entity
macro _ProcessCollision(ns) {
    assert16a()
    assert16i()

    assert(Format.EntityHitboxType._N_BITS == 4)

    lda.l   Format.EntityHitbox.Inner.type,x
    asl
    asl
    asl
    asl
    ldx.w   {ns}.hitboxAddress
    ora.l   Format.EntityHitbox.Inner.type,x
    sta.w   MetaSprite.Collision.bits

    ldy.w   {ns}.entityAddress
    sty.w   MetaSprite.Collision.otherEntity

    // A = collision bits
    // Y = otherEntity
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.ProcessCollision,x)


    // Swap DP and otherEntity
    tdc
    tay
    lda.w   MetaSprite.Collision.otherEntity
    sty.w   MetaSprite.Collision.otherEntity
    tcd

    assert(Format.EntityHitboxType._N_BITS == 4)

    // swap Collision.bits nibbles

    // ::KUDOS Dragonstomper on the AtariAge Forums::
    // http://atariage.com/forums/topic/71120-6502-killer-hacks/page-6#entry1689548

    sep     #$20
a8()
    lda.w   MetaSprite.Collision.bits
    asl
    adc.b   #0x80
    rol
    asl
    adc.b   #0x80
    rol
    sta.w   MetaSprite.Collision.bits


    rep     #$20
a16()
    // DP = other entity
    // Y = this entity
    // A = collision bits
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.ProcessCollision,x)


    // Restore DP
    lda.w   Collision.otherEntity
    tcd
}


namespace _TestList {
NoHitbox:
    rts
}

// Preforms a collision test between the current entity and an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity to test
//         Y = first item in entity list (NOT NULL)
a16()
i16()
code()
function _TestList {
    ldx.b   BaseEntity.MetaSprite.currentFrame
    beq     NoHitbox

    lda.l   Format.Frame.entityHitbox,x
    beq     NoHitbox
    tax

    _SetupOuterAabb(entityA.outer)


    // remember current entity
    tdc
    sta.w   entityA.entityAddress

    // X = EntityHitbox Address
    stx.w   entityA.hitboxAddress

    lda.l   Format.EntityHitbox.count,x
    and.w   #Format.EntityHitbox.COUNT_MASK
    beq     _TestList_EntityA_SingleHitbox
    jmp     _TestList_EntityA_MultipleHitboxes
}



namespace _TestList_EntityA_SingleHitbox {
    NoEntityOuterCollision:
        // continue loop

        lda.b   BaseEntity.next
        bne     EntityLoop

    // restore DP
    lda.w   entityA.entityAddress
    tcd

    rts
}

// Processes the collision tests where entityA has only one hitbox.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entityA address
//         Y = first item in entity list (NOT NULL)
//        entityA values set
// RETURN: DP = entityA address
a16()
i16()
code()
function _TestList_EntityA_SingleHitbox {
    // Current entity only has a single hitbox
    // y = first entity in entityB list
    tya
    EntityLoop:
        tcd

        // DP = entityB address

        ldx.b   BaseEntity.MetaSprite.currentFrame
        beq     NoEntityOuterCollision

        lda.l   Format.Frame.entityHitbox,x
        beq     NoEntityOuterCollision
        tax

        _BranchIfNoOuterCollision(entityA.outer, NoEntityOuterCollision)


        // Outer hitboxes collided
        // Determine number of inner hitboxes in the entity being tested

        // X = EntityHitbox Address
        lda.l   Format.EntityHitbox.count,x
        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     InnerCollision


        // entity being tested has multiple hitboxes

        tay
        InnerLoop:
            // Test inner hitbox matches with single outer

            _BranchOnInnerCollision(entityA.outer, InnerCollision)

        NoInnerCollision:
            // count is number of inner hitboxes - 1
            dey
            bmi     EntityLoop_Continue

            txa
            clc
            adc.w   #Format.EntityHitbox.Inner.size
            tax

            bra     InnerLoop


        InnerCollision:
            // DP = entityB entity
            // X = entityB inner collision

            _ProcessCollision(entityA)


    EntityLoop_Continue:
        lda.b   BaseEntity.next
        beq     EndEntityLoop
            jmp     EntityLoop

EndEntityLoop:
    // restore DP
    lda.w   entityA.entityAddress
    tcd

    rts
}


// Processes the collision tests where entityA has multiple hitboxes.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT:  A = number of hitboxes
//         Y = first item in entity list (NOT NULL)
//         entityA values set
// RETURN: DP = entityA address
a16()
i16()
code()
function _TestList_EntityA_MultipleHitboxes {
    sta.w   entityA.hitboxCount

    // Current entity only has multiple hitbox
    // y = first entity in entityB list
    tya
    EntityLoop:
        tcd

        // DP = entityB

        ldx.b   BaseEntity.MetaSprite.currentFrame
        beq     NoEntityOuterCollision

        lda.l   Format.Frame.entityHitbox,x
        beq     NoEntityOuterCollision
        tax

        _BranchIfNoOuterCollision(entityA.outer, NoEntityOuterCollision)

        // Outer hitboxes collided
        // Determine number of inner hitboxes in the entity being tested

        // X = entityB EntityHitbox Address
        tdc
        sta.w   entityB.entityAddress
        stx.w   entityB.hitboxAddress

        lda.l   Format.EntityHitbox.count,x
        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     _InnerTest_Multiple_Single
        jmp     _InnerTest_Multiple_Multiple

    InnerTest_Return:
        // restore dp
        lda.w   entityB.entityAddress
        tcd

    NoEntityOuterCollision:
        lda.b   BaseEntity.next
        bne     EntityLoop


    // restore DP
    lda.w   entityA.entityAddress
    tcd

    rts


// INPUT: DP = entityB address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
// RETURN: InnerTest_Return
a16()
i16()
code()
function _InnerTest_Multiple_Single {
    // entityA has multiple hitboxes
    // entityB has a single hitbox

    _SetupOuterAabb(entityB.aabb)

    lda.w   entityA.entityAddress
    tcd
    ldy.w   entityA.hitboxCount
    ldx.w   entityA.hitboxAddress

    // DP = entityA

    InnerLoop:
        _BranchOnInnerCollision(entityB.aabb, InnerCollision)

        // count is number of inner hitboxes - 1
        dey
        bmi     Return

        txa
        clc
        adc.w   #Format.EntityHitbox.Inner.size
        tax

        bra     InnerLoop


    InnerCollision:
        // DP = entityA
        // X = entityA inner collision
        // entityB.entityAddress and entityB.hitboxAddress filled

        _ProcessCollision(entityB)

Return:
    jmp     InnerTest_Return
}


// INPUT: A = number of hitboxes in entityB
//        DP = entityB address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
// RETURN: InnerTest_Return
a16()
i16()
code()
function _InnerTest_Multiple_Multiple {
allocateTmpWord(entityB_xPos)
allocateTmpWord(entityB_yPos)
allocateTmpWord(collisionTableIndex)
allocateTmpWord(tmp_row)

constant NoCollisions = _InnerTest_Multiple_Single.Return

    assert(Tables.EntityHitbox.MAX_INNER_HITBOXES - 1 == Format.EntityHitbox.COUNT_MASK)
    assert(Tables.EntityHitbox.INNER_OFFSET == Format.EntityHitbox.Inner.size)

    // entityA and entityB have multiple hitboxes

    ldy.b   BaseEntity.xPos.px
    sty.w   entityB_xPos

    ldy.b   BaseEntity.yPos.px
    sty.w   entityB_yPos


    // A = entityB hitbox count
    // assume entityB is > 1
    assert(Tables.EntityHitbox.CollisionOrderTable.SHIFT == 2)
    asl
    asl
    ora.w   entityA.hitboxCount
    asl
    tax

    lda.l   Tables.EntityHitbox.CollisionOrderTable - 2 * 4,x
    tay

    lda.w   entityA.entityAddress
    tcd

    // DP = entityA

    InnerLoop:
        tyx
        lda.l   Tables.EntityHitbox.CollisionOrder,x
        bmi     NoCollisions

        sta.w   tmp_row

        and.w   #0xff
        clc
        adc.w   entityB.hitboxAddress
        tax

        lda.l   Format.EntityHitbox.Inner.xOffset,x
        and.w   #0xff
        clc
        adc.w   entityB_xPos
        sta.w   entityB.aabb.xPos

        lda.l   Format.EntityHitbox.Inner.width,x
        and.w   #0xff
        sta.w   entityB.aabb.width


        lda.l   Format.EntityHitbox.Inner.yOffset,x
        and.w   #0xff
        clc
        adc.w   entityB_yPos
        sta.w   entityB.aabb.yPos

        lda.l   Format.EntityHitbox.Inner.height,x
        and.w   #0xff
        sta.w   entityB.aabb.height


        lda.w   tmp_row + 1
        and.w   #0xff
        clc
        adc.w   entityA.hitboxAddress
        tax

        _BranchOnInnerCollision(entityB.aabb, InnerCollision)

        iny
        iny
        jmp     InnerLoop

InnerCollision:
    // DP = entityA
    // X = entityA inner collision

    // calculate entityB hitboxAddress
    lda.w   tmp_row
    and.w   #0xff
    clc
    adc.w   entityB.hitboxAddress
    sta.w   entityB.hitboxAddress

    jmp     _InnerTest_Multiple_Single.InnerCollision
}
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

