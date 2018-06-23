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

// PUBLIC API
// ==========

// Preforms a collision tests between two entity lists.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// PARAM: aList - first entity list
// PARAM: bList - second entity list
macro TestCollisions(aList, bList) {
    assert16a()
    assert16i()

    ldx.w   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList}, 1)
EndIf{#}:
}


// Preforms a collision test an entity list and multiple entity lists
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// PARAM: aList - first entity list
// PARAM: bList1 : bListN - entity lists to test collisison with aList
macro TestCollisions(aList, bList1, bList2) {
    assert16a()
    assert16i()

    ldx.w   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
EndIf{#}:
}

macro TestCollisions(aList, bList1, bList2, bList3) {
    assert16a()
    assert16i()

    ldx.w   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList3}, 0)
EndIf{#}:
}

macro TestCollisions(aList, bList1, bList2, bList3, bList4) {
    assert16a()
    assert16i()

    ldx.w   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList3}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList4}, 0)
EndIf{#}:
}

macro TestCollisions(aList, bList1, bList2, bList3, bList4, bList5) {
    assert16a()
    assert16i()

    ldx.w   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList3}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList4}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList5}, 0)
EndIf{#}:
}


// VARIABLES
// =========

// Temporary store for the offset/size pair from EntityHitbox struct
// Used to convert width/height from uint8 to uint16
// (3x uint8_t, third byte always 0)
allocate(_offsetSizePair, wram7e, 3)

// The outer loop entity
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

// The inner loop list
allocateTmpWord(listB)

// The inner loop entity
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


// PRIVATE API
// ===========


// Preforms a collision test between two entity lists
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// PARAM: aList - the first entityList
// PARAM: bList - the second entityList
// PARAM: isFirst - non-zero if the this is the first test against aList
// INPUT: X = first entity in aList (MUST NOT BE NULL) (only used if isFirst is true)
inline _TestTwoEntityLists(aList, bList, evaluate isFirst) {
    assert16a()
    assert16i()

    if Entity.lists.{aList} == Entity.lists.{bList} {
        error "Cannot preform a collision test between the same list ({aList} and {bList})"
    }

    ldy.w   Entity.lists.{bList}
    beq     EndIf{#}
        if {isFirst} == 0 {
            ldx.w   Entity.lists.{aList}
        }
        jsr     MetaSprite.EntityHitbox._TestTwoEntityLists
    EndIf{#}:
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
// MODIFIES: DP
// OUTPUT: Collision.otherEntity = original value of DP
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
}


// Preforms a collision test between two entity lists
//
// REQUIRE: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
//
// INPUT: X = first item in listA (NOT NULL)
// INPUT: Y = first item in listB (NOT NULL)
a16()
i16()
code()
function _TestTwoEntityLists {
    sty.w   listB


    OuterLoop:
        // X = entityA

        txy
        // Y = entityA

        ldx.w   BaseEntity.MetaSprite.currentFrame,y
        beq     NoHitbox

        lda.l   Format.Frame.entityHitbox,x
        beq     NoHitbox
        tax

        sty.w   entityA.entityAddress
        stx.w   entityA.hitboxAddress

        tya
        tcd

        _SetupOuterAabb(entityA.outer)

        ldy.w   listB

        lda.l   Format.EntityHitbox.count,x
        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     _TestList_EntityA_SingleHitbox
        jmp     _TestList_EntityA_MultipleHitboxes

    Next_EntityA:
        // DP = unknown

        ldy.w   entityA.entityAddress

    NoHitbox:
        // Y = entityA

        ldx.w   BaseEntity.next,y
        bne     OuterLoop


    lda.w   #0
    tcd

    rts
}


namespace _TestList_EntityA_SingleHitbox {
    NoEntityOuterCollision:
        // continue loop

        lda.b   BaseEntity.next
        bne     EntityLoop

    jmp     Next_EntityA
}

// Processes the collision tests where entityA has only one hitbox.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: dp = entityA address
//         Y = first item in entity list (NOT NULL)
//        entityA values set
// MODIFIES: DP
// RETURN: Jump to _TestTwoEntityLists.Next_EntityA
a16()
i16()
code()
function _TestList_EntityA_SingleHitbox {
    constant Next_EntityA = _TestTwoEntityLists.Next_EntityA

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

            // Restore DP
            lda.w   Collision.otherEntity
            tcd


    EntityLoop_Continue:
        lda.b   BaseEntity.next
        beq     EndEntityLoop
            jmp     EntityLoop

EndEntityLoop:
    jmp     Next_EntityA
}


// Processes the collision tests where entityA has multiple hitboxes.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT:  A = number of hitboxes
//         Y = first item in entity list (NOT NULL)
//         entityA values set
// MODIFIES: DP
// RETURN: Jump to _TestTwoEntityLists.Next_EntityA
a16()
i16()
code()
function _TestList_EntityA_MultipleHitboxes {
    constant Next_EntityA = _TestTwoEntityLists.Next_EntityA

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


    Continue_EntityLoop:
    NoEntityOuterCollision:
        // DP = entityB

        lda.b   BaseEntity.next
        bne     EntityLoop

    jmp     Next_EntityA



// INPUT: DP = entityB address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
//
// RETURN: Jump to InnerCollision if entityA and entityB collided
//         Jump to NoCollisions if no collisions occurred
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
        bmi     NoCollisions

        txa
        clc
        adc.w   #Format.EntityHitbox.Inner.size
        tax

        bra     InnerLoop
}


// Collision occurred between entityA and entityB
// DP = entityA
// X = entityA inner collision
// entityB.entityAddress and entityB.hitboxAddress filled
//
// OUTPUT: DP = entityB
// RETURN: Jump to Continue_EntityLoop
InnerCollision:
    _ProcessCollision(entityB)

NoCollisions:
    lda.w   entityB.entityAddress
    tcd

    jmp     Continue_EntityLoop


// INPUT: A = number of hitboxes in entityB
//        DP = entityB address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
//
// RETURN: Jump to InnerCollision if entityA and entityB collided
//         Jump to NoCollisions if no collisions occurred
a16()
i16()
code()
function _InnerTest_Multiple_Multiple {
allocateTmpWord(entityB_xPos)
allocateTmpWord(entityB_yPos)
allocateTmpWord(collisionTableIndex)
allocateTmpWord(tmp_row)

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

        _BranchOnInnerCollision(entityB.aabb, _InnerCollision)

        iny
        iny
        jmp     InnerLoop


_InnerCollision:
    // DP = entityA
    // X = entityA inner collision

    // calculate entityB hitboxAddress

    lda.w   tmp_row
    and.w   #0xff
    clc
    adc.w   entityB.hitboxAddress
    sta.w   entityB.hitboxAddress

    jmp     InnerCollision
}
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

