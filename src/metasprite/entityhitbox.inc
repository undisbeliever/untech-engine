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

namespace currentEntity {
    // address of the current entity
    allocateTmpWord(entityAddress)

    // address of the entity hitbox
    allocateTmpWord(hitboxAddress)

    // Outer hitbox of current entity
    // All values are offset by INT_MS8_OFFSET
    // (int_ms8_word)
    namespace outer {
        allocateTmpWord(xPos)
        allocateTmpWord(yPos)
        allocateTmpWord(width)
        allocateTmpWord(height)
    }
}

namespace toTestEntity {
    allocateTmpWord(entityAddress)
    allocateTmpWord(hitboxAddress)

    namespace aabb {
        allocateTmpWord(xPos)
        allocateTmpWord(yPos)
        allocateTmpWord(width)
        allocateTmpWord(height)
    }
}

allocateTmpWord(tmp_count)



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
// INPUT: dp = current entity
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
    sta.w   {aabb}.width


    lda.l   Format.EntityHitbox.Outer.yOffset,x
    and.w   #0xff
    clc
    adc.b   BaseEntity.yPos.px
    sta.w   {aabb}.yPos

    lda.l   Format.EntityHitbox.Outer.height,x
    sta.w   {aabb}.height
}


// Branches to label if there is no collision between
// the dp entity's outer hitbox and aabb
//
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity to test
// INPUT: X = EntityHitbox address
// KEEP: X, Y
// BRANCH: to `noCollision` if there is no collision
macro _BranchIfNoOuterCollision(aabb, NoCollision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        lda.l   Format.EntityHitbox.Outer.xOffset,x
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px

        cmp.w   {aabb}.xPos
        bcs     XAhead
            // c clear
            adc.l   Format.EntityHitbox.Outer.width,x
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
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px
        cmp.w   {aabb}.yPos
        bcs     YAhead
            // c clear
            adc.l   Format.EntityHitbox.Outer.height,x
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
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px

        cmp.w   {aabb}.xPos
        bcs     XAhead
            // c clear
            adc.l   Format.EntityHitbox.Inner.width,x
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
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px
        cmp.w   {aabb}.yPos
        bcs     YAhead
            // c clear
            adc.l   Format.EntityHitbox.Inner.height,x
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

    _SetupOuterAabb(currentEntity.outer)


    // remember current entity
    tdc
    sta.w   currentEntity.entityAddress

    // X = EntityHitbox Address
    stx.w   currentEntity.hitboxAddress

    lda.l   Format.EntityHitbox.count,x
    and.w   #Format.EntityHitbox.COUNT_MASK
    beq     _TestList_CurrentHasSingleHitbox
    jmp     _TestList_CurrentHasMultipleHitboxes
}



namespace _TestList_CurrentHasSingleHitbox {
    NoEntityOuterCollision:
        // continue loop

        lda.b   BaseEntity.next
        bne     EntityLoop

    // restore DP
    lda.w   currentEntity.entityAddress
    tcd

    rts
}

// Processes the collision tests where the current entity
// has only one hitbox.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entity to test
//         Y = first item in entity list (NOT NULL)
//        currentEntity values set
a16()
i16()
code()
function _TestList_CurrentHasSingleHitbox {
    // Current entity only has a single hitbox
    // y = first entity to test
    tya
    EntityLoop:
        tcd

        ldx.b   BaseEntity.MetaSprite.currentFrame
        beq     NoEntityOuterCollision

        lda.l   Format.Frame.entityHitbox,x
        beq     NoEntityOuterCollision
        tax

        _BranchIfNoOuterCollision(currentEntity.outer, NoEntityOuterCollision)


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

            _BranchOnInnerCollision(currentEntity.outer, InnerCollision)

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
            // DP = toTest entity
            // X = toTest inner collision

            assert(Format.EntityHitboxType._TABLE_SHIFT == 3)

            lda.l   Format.EntityHitbox.Inner.type,x
            asl
            asl
            asl
            ldx.w   currentEntity.hitboxAddress
            ora.l   Format.EntityHitbox.Inner.type,x
            and.w   #CollisionTable.TABLE_MASK
            tax

            ldy     currentEntity.entityAddress
            jsr     (CollisionTable.FunctionTable,x)
            bra     EndEntityLoop

    EntityLoop_Continue:
        lda.b   BaseEntity.next
        beq     EndEntityLoop
            jmp     EntityLoop

EndEntityLoop:
    // restore DP
    lda.w   currentEntity.entityAddress
    tcd

    rts
}


// Processes the collision tests where the current entity
// has multiple hitboxes.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT:  A = number of hitboxes
//         Y = first item in entity list (NOT NULL)
//        currentEntity values set
a16()
i16()
code()
function _TestList_CurrentHasMultipleHitboxes {
    sta.w   tmp_count

    // Current entity only has multiple hitbox
    // y = first entity to test
    tya
    EntityLoop:
        tcd

        ldx.b   BaseEntity.MetaSprite.currentFrame
        beq     NoEntityOuterCollision

        lda.l   Format.Frame.entityHitbox,x
        beq     NoEntityOuterCollision
        tax

        _BranchIfNoOuterCollision(currentEntity.outer, NoEntityOuterCollision)

        // Outer hitboxes collided
        // Determine number of inner hitboxes in the entity being tested

        // X = toTest EntityHitbox Address
        tdc
        sta.w   toTestEntity.entityAddress
        stx.w   toTestEntity.hitboxAddress

        lda.l   Format.EntityHitbox.count,x
        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     _InnerTest_Multiple_Single
        jmp     _InnerTest_Multiple_Multiple

    InnerTest_Return:
        // restore dp
        lda.w   toTestEntity.entityAddress
        tcd

    NoEntityOuterCollision:
        lda.b   BaseEntity.next
        bne     EntityLoop


    // restore DP
    lda.w   currentEntity.entityAddress
    tcd

    rts


// INPUT: dp = toTest entity
//        tmp_count = number of hitboxes in current entity
//        currentEntity values set
//        toTestEntity.entityAddress set
//        toTestEntity.hitboxAddress set
// RETURN: InnerTest_Return
a16()
i16()
code()
function _InnerTest_Multiple_Single {
    // current entity has mutliple hitboxes
    // entity being tested has a single hitboxes

    _SetupOuterAabb(toTestEntity.aabb)

    lda.w   currentEntity.entityAddress
    tcd
    ldy.w   tmp_count
    ldx.w   currentEntity.hitboxAddress

    InnerLoop:
        // DP = entity to test
        _BranchOnInnerCollision(toTestEntity.aabb, InnerCollision)

        // count is number of inner hitboxes - 1
        dey
        bmi     Return

        txa
        clc
        adc.w   #Format.EntityHitbox.Inner.size
        tax

        bra     InnerLoop


    InnerCollision:
        // DP = currentEntity
        // X = currentEntity inner collision

        assert(Format.EntityHitboxType._TABLE_SHIFT == 3)

        lda.l   Format.EntityHitbox.Inner.type,x
        asl
        asl
        asl
        ldx.w   toTestEntity.hitboxAddress
        ora.l   Format.EntityHitbox.Inner.type,x
        and.w   #CollisionTable.TABLE_MASK
        tax

        ldy     toTestEntity.entityAddress
        jsr     (CollisionTable.FunctionTable,x)

Return:
    jmp     InnerTest_Return
}


// INPUT: A = number of hitboxes in toTest entity
// INPUT: dp = toTest entity
//        tmp_count = number of hitboxes in current entity
//        currentEntity values set
//        toTestEntity.entityAddress set
//        toTestEntity.hitboxAddress set
// RETURN: InnerTest_Return
a16()
i16()
code()
function _InnerTest_Multiple_Multiple {
allocateTmpWord(toTest_xPos)
allocateTmpWord(toTest_yPos)
allocateTmpWord(collisionTableIndex)
allocateTmpWord(tmp_row)

constant NoCollisions = _InnerTest_Multiple_Single.Return

    // current entity has mutliple hitboxes
    // entity being tested has multiple hitboxes

    ldy.b   BaseEntity.xPos.px
    sty.w   toTest_xPos

    ldy.b   BaseEntity.yPos.px
    sty.w   toTest_yPos


    // A = toTest count
    // assume toTest is > 1
    assert(Tables.EntityHitbox.CollisionOrderTable.SHIFT == 2)
    asl
    asl
    ora.w   tmp_count
    asl
    tax

    lda.l   Tables.EntityHitbox.CollisionOrderTable - 2 * 4,x
    tay

    lda.w   currentEntity.entityAddress
    tcd

    InnerLoop:
        tyx
        lda.l   Tables.EntityHitbox.CollisionOrder,x
        bmi     NoCollisions

        sta.w   tmp_row

        and.w   #0xff
        clc
        adc.w   toTestEntity.hitboxAddress
        tax

        lda.l   Format.EntityHitbox.Inner.xOffset,x
        and.w   #0xff
        clc
        adc.w   toTest_xPos
        sta.w   toTestEntity.aabb.xPos

        lda.l   Format.EntityHitbox.Inner.width,x
        sta.w   toTestEntity.aabb.width


        lda.l   Format.EntityHitbox.Inner.yOffset,x
        and.w   #0xff
        clc
        adc.w   toTest_yPos
        sta.w   toTestEntity.aabb.yPos

        lda.l   Format.EntityHitbox.Inner.height,x
        sta.w   toTestEntity.aabb.height


        lda.w   tmp_row + 1
        and.w   #0xff
        clc
        adc.w   currentEntity.hitboxAddress
        tax

        _BranchOnInnerCollision(toTestEntity.aabb, InnerCollision)

        iny
        iny
        jmp     InnerLoop

InnerCollision:
    // DP = currentEntity
    // X = currentEntity inner collision

    lda.w   tmp_row
    and.w   #0xff
    clc
    adc.w   toTestEntity.hitboxAddress
    tay


    // Y = toTestEntity hitbox address
    assert(Format.EntityHitboxType._TABLE_SHIFT == 3)

    lda.l   Format.EntityHitbox.Inner.type,x
    asl
    asl
    asl

    tyx
    ora.l   Format.EntityHitbox.Inner.type,x
    and.w   #CollisionTable.TABLE_MASK
    tax

    ldy.w   toTestEntity.entityAddress
    jsr     (CollisionTable.FunctionTable,x)

    jmp     InnerTest_Return
}
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

