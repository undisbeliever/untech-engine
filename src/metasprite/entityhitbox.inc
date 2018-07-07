// metasprite/entityhitbox.inc
// ===========================
//
// MetaSprite Entity Hitbox collision tests.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {
namespace EntityHitbox {


// PUBLIC API
// ==========


// Start the EntityHitbox collision test block.
//
// You MUST ONLY invoke `MetaSprite.EntityHitbox.TestCollisions`
// inside the EntityHitbox Begin()-End() block.
//
// You MUST invoke `MetaSprite.EntityHitbox.End` to end the block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
// OUTPUT: DB = EntityHitboxBank
macro Begin() {
    assert16a()
    assert16i()

    if MetaSprite.EntityHitbox.__EH_BLOCK != 0 {
        error "Cannot invoke MetaSprite.EntityHitbox.Begin() twice"
    }
    MetaSprite.EntityHitbox.__EH_BLOCK = 1

    pea     (0x7e << 8) | MetaSprite.EntityHitbox.EntityHitboxBank
    plb

    // Ensure the third byte of the offset/size pair is zero
    stz.b   MetaSprite.EntityHitbox._xPosWidthPair + 1
    stz.b   MetaSprite.EntityHitbox._yPosHeightPair + 1
}



// End the EntityHitbox collision test block.
//
// Restores the DP/DB registers.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// OUTPUT: DB = 0x7e
macro End() {
    if MetaSprite.EntityHitbox.__EH_BLOCK == 0 {
        error "Expected MetaSprite.EntityHitbox.Begin()"
    }
    if MetaSprite.EntityHitbox.__EH_BLOCK == 1 {
        error "Expected at least one MetaSprite.EntityHitbox.TestCollisions()"
    }

    MetaSprite.EntityHitbox.__EH_BLOCK = 0

    plb
}



// Preforms an EntityHitbox collision test between two entity lists.
//
// This macro MUST BE invoked between a `MetaSprite.EntityHitbox.Begin()` and
// `MetaSprite.EntityHitbox.End()` block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// PARAM: aList - first entity list
// PARAM: bList - second entity list
macro TestCollisions(aList, bList) {
    assert16a()
    assert16i()

    MetaSprite.EntityHitbox._TestInBlock()

    // ::TODO move entity lists to dp::

    lda.l   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList}, 1)
EndIf{#}:
}



// Preforms an EntityHitbox collision test, testing for collisions between
// a single entity list with multiple entity lists
//
// This macro MUST BE invoked between a `MetaSprite.EntityHitbox.Begin()` and
// `MetaSprite.EntityHitbox.End()` block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// PARAM: aList - first entity list
// PARAM: bList1 : bListN - entity lists to test collisison with aList
macro TestCollisions(aList, bList1, bList2) {
    assert16a()
    assert16i()

    MetaSprite.EntityHitbox._TestInBlock()

    // ::TODO move entity lists to dp::

    lda.l   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
EndIf{#}:
}

macro TestCollisions(aList, bList1, bList2, bList3) {
    assert16a()
    assert16i()

    MetaSprite.EntityHitbox._TestInBlock()

    lda.l   Entity.lists.{aList}
    beq     EndIf{#}

        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList1}, 1)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList2}, 0)
        MetaSprite.EntityHitbox._TestTwoEntityLists({aList}, {bList3}, 0)
EndIf{#}:
}

macro TestCollisions(aList, bList1, bList2, bList3, bList4) {
    assert16a()
    assert16i()

    MetaSprite.EntityHitbox._TestInBlock()

    lda.l   Entity.lists.{aList}
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

    MetaSprite.EntityHitbox._TestInBlock()

    lda.l   Entity.lists.{aList}
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


// EntityHitbox Begin()-End() block status
variable __EH_BLOCK = 0

// Data bank containing the EntityHitbox data
constant EntityHitboxBank = Data.EntityHitboxOffset >> 16

// Temporary store for the offset/size pair from EntityHitbox struct
// Used to convert width/height from uint8 to uint16
// Also used to store the last hitbox read by the test collision code
// (2x 3x uint8_t, third byte always 0)
allocate(_xPosWidthPair, dp, 3)
allocate(_yPosHeightPair, dp, 3)

// Test assumption made by _xPosWidthPair/_yPosHeightPair optimisation
assert(Format.EntityHitbox.Outer.xOffset + 1 == Format.EntityHitbox.Outer.width)
assert(Format.EntityHitbox.Outer.yOffset + 1 == Format.EntityHitbox.Outer.height)
assert(Format.EntityHitbox.Inner.xOffset + 1 == Format.EntityHitbox.Inner.width)
assert(Format.EntityHitbox.Inner.yOffset + 1 == Format.EntityHitbox.Inner.height)


// The outer loop entity variables
namespace entityA {
    // address of the entity
    // (word addr)
    allocate(entityAddress, dp, 2)

    // address of the entity hitbox
    allocate(hitboxAddress, dp, 2)

    // number of hitboxes
    // (only set in when entity has Multiple hitboxes)
    allocate(hitboxCount, dp, 2)

    // Outer hitbox of entity
    // all values are offset by INT_MS8_OFFSET
    // (4x uint16)
    namespace outer {
        allocate(left, dp, 2)
        allocate(right, dp, 2)
        allocate(top, dp, 2)
        allocate(bottom, dp, 2)
    }
}


// The inner loop list
allocate(listB, dp, 2)

// The inner loop entity variables
namespace entityB {
    // entityAddress must survive a ProcessCollision function call
    // (word addr)
    allocate(entityAddress, dp, 2)

    // address of the entity hitbox
    // (word addr)
    constant hitboxAddress      = dpTmp0

    // Used for both inner and outer hitbox tests
    // all values are offset by INT_MS8_OFFSET
    // (4x uint16)
    namespace aabb {
        constant left           = dpTmp1
        constant right          = dpTmp2
        constant top            = dpTmp3
        constant bottom         = dpTmp4
    }
}

// Temporary Variables

constant _count                 = dpTmp5
constant _collisonOrderPos      = dpTmp6
constant _row                   = dpTmp7
constant _collisionTableIndex   = dpTmp8

constant _entityA_xPos          = dpTmp9
constant _entityA_yPos          = dpTmp10
constant _entityB_xPos          = dpTmp11
constant _entityB_yPos          = dpTmp12

constant _tmp                   = dpTmp13



// PRIVATE API
// ===========


// Tests that we are in a EntityHitbox Begin()-End() block
inline _TestInBlock() {
    if MetaSprite.EntityHitbox.__EH_BLOCK == 0 {
        error "Expected MetaSprite.EntityHitbox.Begin()"
    }
    MetaSprite.EntityHitbox.__EH_BLOCK = MetaSprite.EntityHitbox.__EH_BLOCK + 1
}



// Preforms the EntityHitbox collision tests between two entity lists.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// PARAM: aList - the first entityList
// PARAM: bList - the second entityList (MUST NOT BE aList)
// PARAM: isFirst - non-zero if the this is the first test against aList
// INPUT: A = first entity in aList (MUST NOT BE NULL) (only used if isFirst is true)
inline _TestTwoEntityLists(aList, bList, evaluate isFirst) {
    assert16a()
    assert16i()

    if Entity.lists.{aList} == Entity.lists.{bList} {
        error "Cannot preform a collision test between the same list ({aList} and {bList})"
    }

    // ::TODO move entity lists to dp::

    lda.l   Entity.lists.{bList}
    beq     EndIf{#}
        tay
        lda.l   Entity.lists.{aList}
//        if {isFirst} == 0 {
//            lda.l   Entity.lists.{aList}
//        }
        jsr     MetaSprite.EntityHitbox._TestTwoEntityLists
    EndIf{#}:
}



// Calculates the aabb of an entity's outer hitbox.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: X = entity
//        Y = EntityHitbox address
// OUTPUT: _xPosWidthPair/_yPosHeightPair = the outer hitbox data
macro _SetupOuterAabb(aabb) {
    assert16a()
    assert16i()

    lda.w   Format.EntityHitbox.Outer.xOffset,y
    sta.b   _xPosWidthPair

    and.w   #0xff
    clc
    adc.b   BaseEntity.xPos.px,x
    sta.b   {aabb}.left

    sec                             // + 1
    adc.b   _xPosWidthPair + 1      // width
    sta.b   {aabb}.right


    lda.w   Format.EntityHitbox.Outer.yOffset,y
    sta.b   _yPosHeightPair

    and.w   #0xff
    clc
    adc.b   BaseEntity.yPos.px,x
    sta.b   {aabb}.top

    sec                             // + 1
    adc.b   _yPosHeightPair + 1     // height
    sta.b   {aabb}.bottom
}



// Tests for a collision between an entity's outer hitbox and a given aabb,
// branching to a given label if no collision occurred.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: X = entity
//        Y = EntityHitbox address
// KEEP: X, Y
// OUTPUT: _xPosWidthPair/_yPosHeightPair = the outer hitbox data (if a collision occurred)
// BRANCH: to `noCollision` if there is no collision
macro _BranchIfNoOuterCollision(aabb, NoCollision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        // if left >= aabb.right
        //    || right <= aabb.left: goto NoCollision

        lda.w   Format.EntityHitbox.Outer.xOffset,y
        sta.b   _xPosWidthPair
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px,x

        cmp.b   {aabb}.right
        bcs     {NoCollision}

        // c clear
        adc.b   _xPosWidthPair + 1      // width
        // A = right - 1
        cmp.b   {aabb}.left
        bcc     {NoCollision}


        lda.w   Format.EntityHitbox.Outer.yOffset,y
        sta.b   _yPosHeightPair
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px,x

        cmp.b   {aabb}.bottom
        bcs     {NoCollision}

        // c clear
        adc.b   _yPosHeightPair + 1     // height
        // A = bottom - 1
        cmp.b   {aabb}.top
        bcc     {NoCollision}
    }
}



// Tests for a collision between an entity's inner hitbox and a given aabb,
// branching to a given label if the collision occurred.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: X = entity to test
//        Y = EntityHitbox address + inner hitbox offset
// KEEP: X, Y
// OUTPUT: _xPosWidthPair/_yPosHeightPair = the inner hitbox data (if a collision occurred)
// BRANCH: to `Collision` if there is a collision
macro _BranchOnInnerCollision(aabb, Collision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        // if left >= aabb.right
        //    || right <= aabb.left: goto NoCollision

        lda.w   Format.EntityHitbox.Inner.xOffset,y
        sta.b   _xPosWidthPair
        and.w   #0xff
        clc
        adc.b   BaseEntity.xPos.px,x

        cmp.b   {aabb}.right
        bcs     NoCollision

        // c clear
        // Deliberately not one to right
        adc.b   _xPosWidthPair + 1      // width
        // A = right - 1
        cmp.b   {aabb}.left
        bcc     NoCollision


        lda.w   Format.EntityHitbox.Inner.yOffset,y
        sta.b   _yPosHeightPair
        and.w   #0xff
        clc
        adc.b   BaseEntity.yPos.px,x

        cmp.b   {aabb}.bottom
        bcs     NoCollision

        // c clear
        adc.b   _yPosHeightPair + 1     // height
        // A = bottom - 1
        cmp.b   {aabb}.top
        bcs     {Collision}

    NoCollision:
    }
}



// Tests for a collision between two inner hitboxes at two given positions,
// branching to a given label if the collision occurred.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: Y = entityA hitbox + inner hitbox offset
//        X = entityB hitbox + inner hitbox offset
//        _entityA_xPos, _entityA_yPos = entity A position
//        _entityB_xPos, _entityB_yPos = entity B position
// KEEP: X, Y
// OUTPUT: entityB.aabb = entityB inner hitbox (if a collision occurred)
// OUTPUT: _xPosWidthPair/_yPosHeightPair = entityA inner hitbox data (if a collision occurred)
// BRANCH: to `Collision` if there is a collision
macro _BranchOnInnerInnerCollision(Collision) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    namespace {#} {
        // Fill entityB.aabb X axis

        lda.w   Format.EntityHitbox.Inner.xOffset,x
        sta.b   _xPosWidthPair
        and.w   #0xff
        clc
        adc.b   _entityB_xPos
        sta.b   entityB.aabb.left

        sec                             // + 1
        adc.b   _xPosWidthPair + 1      // width
        sta.b   entityB.aabb.right


        // if left >= aabb.right
        //    || right <= aabb.left: goto NoCollision

        lda.w   Format.EntityHitbox.Inner.xOffset,y
        sta.b   _xPosWidthPair
        and.w   #0xff
        clc
        adc.b   _entityA_xPos

        cmp.b   entityB.aabb.right
        bcs     NoCollision

        // c clear
        // Deliberately not one to right
        adc.b   _xPosWidthPair + 1      // width
        // A = entityA right - 1
        cmp.b   entityB.aabb.left
        bcc     NoCollision


        // Fill entityB.aabb Y axis

        lda.w   Format.EntityHitbox.Inner.yOffset,x
        sta.b   _yPosHeightPair
        and.w   #0xff
        clc
        adc.b   _entityB_yPos
        sta.b   entityB.aabb.top

        sec                             // + 1
        adc.b   _yPosHeightPair + 1     // height
        sta.b   entityB.aabb.bottom


        // Test y axis

        lda.w   Format.EntityHitbox.Inner.yOffset,y
        sta.b   _yPosHeightPair
        and.w   #0xff
        clc
        adc.b   _entityA_yPos

        cmp.b   entityB.aabb.bottom
        bcs     NoCollision

        // c clear
        adc.b   _yPosHeightPair + 1     // height
        // A = entityA bottom - 1
        cmp.b   entityB.aabb.top
        bcs     {Collision}

    NoCollision:
    }
}



// Calculates the centre of the collision, storing it in `MetaSprite.Collision`.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// INPUT: aabb = hitbox used in last test
// INPUT: X = entity
// INPUT: _xPosWidthPair/_yPosHeightPair = entity hitbox data of last test
// KEEP: X/Y
// OUTPUT: MetaSprite.Collision.xPos, MetaSprite.Collision.yPos
macro _CalculateCollisionCentre(aabb) {
    assert16a()
    assert16i()

    __CalculateCollisionCentreAxis(xPos, _xPosWidthPair, {aabb}.left, {aabb}.right)
    __CalculateCollisionCentreAxis(yPos, _yPosHeightPair, {aabb}.top, {aabb}.bottom)
}

// Calculates the centre of the collision for a single axis.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// PARAM: axis = xPos/yPos
// PARAM: offsetSizePair = _xPosWidthPair/_yPosHeightPair
// PARAM: a1 = left/top
// PARAM: a2 = right/bottom
//
// INPUT: X = entity
// KEEP: X/Y
// OUTPUT: MetaSprite.Collision.{axis}
inline __CalculateCollisionCentreAxis(axis, offsetSizePair, a1, a2) {
    assert16a()
    assert16i()

    assert(MetaSprite.Collision.{axis} < 0x100)

    // b1 = offset + entity.{axis}
    // b2 = b1 + size + 1
    // axis = avg(max(a1, b1), min(a2, b2)) - INT_MS8_OFFSET

    lda.b   {offsetSizePair}
    and.w   #0x00ff
    clc
    adc.b   BaseEntity.{axis}.px,x
    sta.b   _tmp

    cmp.b   {a1}
    bcs     _Max{#}
        lda.b   {a1}
        sec
_Max{#}:
    sta.b   MetaSprite.Collision.{axis}

    // c set                        // + 1
    lda.b   _tmp
    adc.b   {offsetSizePair} + 1    // width/height
    cmp.b   {a2}
    bcc     _Min{#}
        lda.b   {a2}
        clc
_Min{#}:

    // c clear
    adc.b   MetaSprite.Collision.{axis}
    ror
    sec
    sbc.w   #INT_MS8_OFFSET
    sta.b   MetaSprite.Collision.{axis}
}



// Process the collision.
//
// Calls BaseEntityFunctionTable.ProcessCollision for both entities with the appropriate
// collisionBits.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// INPUT:  X = this entity address
//         Y = this hitbox address + inner hitbox offset
//         ns = namespace holding the other entity
//         MetaSprite.Collision.xPos, MetaSprite.Collision.yPos = collision centre
// OUTPUT: Collision.otherEntity = original value of X
macro _ProcessCollision(ns) {
    assert16a()
    assert16i()

    // Ensure this macro can be used by both single and multiple hitbox data
    assert(Format.EntityHitbox.singleHitboxType == Format.EntityHitbox.Inner.type)

    assert(Format.EntityHitboxType._N_BITS == 4)

    txa
    ldx.b   {ns}.hitboxAddress
    tcd
    // DP = entity
    // Y = entity inner hitbox
    // X = other entity inner hitbox

    lda.w   Format.EntityHitbox.Inner.type,y
    asl
    asl
    asl
    asl
    ora.w   Format.EntityHitbox.Inner.type,x

    pea     (EntityHitboxBank << 8) | 0x7e
    plb
    // DB = 0x7e

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


    // Restore DP and DB
    lda.w   #0
    tcd
    // DP = 0
    plb
    // DB = EntityHitboxBank
}



// Preforms an EntityHitbox collision test between two entity lists.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
//
// INPUT: A = first item in listA (NOT NULL)
// INPUT: Y = first item in listB (NOT NULL, MUST NOT BE listA)
a16()
i16()
code()
function _TestTwoEntityLists {
    sty.b   listB

    // A = entityA
    OuterLoop:
        tay
        // A & Y = entityA

        ldx.b   BaseEntity.MetaSprite.currentFrame,y
        beq     NoHitbox

        if Data.FrameList >> 16 == EntityHitboxBank {
            ldy.w   Format.Frame.entityHitbox,x
            beq     NoHitbox

            tax
        } else {
            lda.l   Format.Frame.entityHitbox,x
            beq     NoHitbox

            tyx
            tay
        }

        // X = entityAddress
        // Y = hitboxAddress

        stx.b   entityA.entityAddress
        sty.b   entityA.hitboxAddress

        _SetupOuterAabb(entityA.outer)

        lda.w   Format.EntityHitbox.count,y
        ldy.b   listB

        // Y = listB, A = unmasked count

        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     _TestList_EntityA_SingleHitbox
        jmp     _TestList_EntityA_MultipleHitboxes


    NoHitbox:
        // Y = entityA
        tyx
        lda.b   BaseEntity.next,x
        bne     OuterLoop

        rts


    Next_EntityA:
        // cannot use `lda (entityA.entityAddress)` here as DB cannot access shadow
        // Doesn't matter as it is only one cycle slower

        ldx.b   entityA.entityAddress
        lda.b   BaseEntity.next,x
        bne     OuterLoop

        rts
}



// Preforms the EntityHitbox collision tests between entityA and listB, where
// entityA has only one hitbox.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: Y = first entity in listB (NOT NULL)
//        entityA values set
// RETURN: Jump to _TestTwoEntityLists.Next_EntityA
a16()
i16()
code()
namespace _TestList_EntityA_SingleHitbox {
    constant Next_EntityA = _TestTwoEntityLists.Next_EntityA

    NoEntityOuterCollision_Y:
        // Y = entityB address
        tyx

    NoEntityOuterCollision_X:
        // X = entityB address

        ldy.b   BaseEntity.next,x
        beq     Next_EntityA

START:
    EntityLoop:
        // Y = entityB address

        ldx.b   BaseEntity.MetaSprite.currentFrame,y
        beq     NoEntityOuterCollision_Y

        if Data.FrameList >> 16 == EntityHitboxBank {
            lda.w   Format.Frame.entityHitbox,x
        } else {
            lda.l   Format.Frame.entityHitbox,x
        }
        beq     NoEntityOuterCollision_Y
        tyx
        tay

        // X = entity address
        // Y = hitbox address

        _BranchIfNoOuterCollision(entityA.outer, NoEntityOuterCollision_X)


        // Outer hitboxes collided
        // Determine number of inner hitboxes in the entity being tested

        // Y = EntityHitbox Address
        lda.w   Format.EntityHitbox.count,y
        and.w   #Format.EntityHitbox.COUNT_MASK
        beq     InnerCollision


        // entity being tested has multiple hitboxes
        sta.b   _count


        bra     InnerLoop_FIRST


        InnerLoop:
            // increment inner hitbox offset
            tya
            clc
            adc.w   #Format.EntityHitbox.Inner.size
            tay

        InnerLoop_FIRST:
            // Test inner hitbox matches with single outer

            _BranchOnInnerCollision(entityA.outer, InnerCollision)

            // count is number of inner hitboxes - 1
            dec.b   _count
            bpl     InnerLoop


    EntityLoop_Continue:
        // X = entityB address
        ldy.b   BaseEntity.next,x
        bne     EntityLoop

        jmp     Next_EntityA


    InnerCollision:
        // X = entityB address
        // Y = entityB hitbox address + inner hitbox offset

        _CalculateCollisionCentre(entityA.outer)

        _ProcessCollision(entityA)

        // entityB stored in Collision.otherEntity

        assert(Collision.otherEntity < 0x100)
        ldx.b   Collision.otherEntity
        jmp     EntityLoop_Continue
}
constant _TestList_EntityA_SingleHitbox = _TestList_EntityA_SingleHitbox.START



// Preforms the EntityHitbox collision tests between entityA and listB, where
// entityA has multiple hitboxes.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: Y = first entity in listB (NOT NULL)
//        entityA values set
// RETURN: Jump to _TestTwoEntityLists.Next_EntityA
a16()
i16()
code()
function _TestList_EntityA_MultipleHitboxes {
    constant Next_EntityA = _TestTwoEntityLists.Next_EntityA

    // Current entity only has multiple hitboxex

    sta.b   entityA.hitboxCount

    EntityLoop:
        // Y = entityB

        ldx.b   BaseEntity.MetaSprite.currentFrame,y
        beq     NoEntityOuterCollision_Y

        if Data.FrameList >> 16 == EntityHitboxBank {
            lda.w   Format.Frame.entityHitbox,x
        } else {
            lda.l   Format.Frame.entityHitbox,x
        }
        beq     NoEntityOuterCollision_Y

        tyx
        tay

        // X = entityB
        // Y = entityB hitbox

        _BranchIfNoOuterCollision(entityA.outer, NoEntityOuterCollision_X)

            // Outer hitboxes collided
            // Determine number of inner hitboxes in the entity being tested

            stx.b   entityB.entityAddress
            sty.b   entityB.hitboxAddress

            lda.w   Format.EntityHitbox.count,y
            and.w   #Format.EntityHitbox.COUNT_MASK
            beq     _InnerTest_Multiple_Single
            jmp     _InnerTest_Multiple_Multiple


    NoEntityOuterCollision_Y:
        tyx

    NoEntityOuterCollision_X:
        // X = entityB
        ldy.b   BaseEntity.next,x
        bne     EntityLoop
        jmp     Next_EntityA


    Continue_EntityLoop:
        // cannot use `lda (entityB.entityB)` here as DB cannot access shadow

        ldx.b   entityB.entityAddress
        ldy.b   BaseEntity.next,x
        bne     EntityLoop
        jmp     Next_EntityA



// Preforms the EntityHitbox collision test between entityA and entityB, where
// entityA has multiple hitboxes, entityB has only one hitbox and both
// entities' outer hitboxes collided.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: X = entityB address
//        Y = entityB hitbox address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
//
// RETURN: Jump to InnerCollision if entityA and entityB collided
//         Jump to Continue_EntityLoop if no collisions occurred
a16()
i16()
code()
function _InnerTest_Multiple_Single {
    // entityA has multiple hitboxes
    // entityB has a single hitbox

    _SetupOuterAabb(entityB.aabb)

    ldx.b   entityA.entityAddress
    ldy.b   entityA.hitboxAddress

    lda.b   entityA.hitboxCount
    sta.b   _count


    bra     InnerLoop_FIRST


    InnerLoop:
        // increment inner hitbox offset
        tya
        clc
        adc.w   #Format.EntityHitbox.Inner.size
        tay

    InnerLoop_FIRST:
        // X = entityA
        // Y = entityA hitbox

        _BranchOnInnerCollision(entityB.aabb, InnerCollision)

        // count is number of inner hitboxes - 1
        dec.b   _count
        bpl     InnerLoop

    bra     Continue_EntityLoop
}



// Process the inner collision between entityA and entityB, where entityA has
// multiple hitboxes.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: X = entityA address
//        Y = entityA hitbox address + inner hitbox offset
//        entityB.entityAddress set
//        entityB.hitboxAddress = entityB hitbox address + inner hitbox offset
//
// RETURN: Jump to Continue_EntityLoop
a16()
i16()
code()
function InnerCollision {
    _CalculateCollisionCentre(entityB.aabb)

    _ProcessCollision(entityB)

    jmp     Continue_EntityLoop
}



// Preforms the EntityHitbox collision test between entityA and entityB, where
// both entities have multiple hitboxes and both entities' outer hitboxes
// collided.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = EntityHitboxBank
// INPUT: A = number of hitboxes in entityB
//        X = entityB address
//        Y = entityB hitbox address
//        entityA values set
//        entityB.entityAddress set
//        entityB.hitboxAddress set
//
// RETURN: Jump to InnerCollision if entityA and entityB collided
//         Jump to Continue_EntityLoop if no collisions occurred
a16()
i16()
code()
function _InnerTest_Multiple_Multiple {

    assert(Tables.EntityHitbox.MAX_INNER_HITBOXES - 1 == Format.EntityHitbox.COUNT_MASK)
    assert(Tables.EntityHitbox.INNER_OFFSET == Format.EntityHitbox.Inner.size)

    // entityA and entityB have multiple hitboxes

    ldy.b   BaseEntity.xPos.px,x
    sty.b   _entityB_xPos

    ldy.b   BaseEntity.yPos.px,x
    sty.b   _entityB_yPos


    ldx.b   entityA.entityAddress

    ldy.b   BaseEntity.xPos.px,x
    sty.b   _entityA_xPos

    ldy.b   BaseEntity.yPos.px,x
    sty.b   _entityA_yPos


    // A = entityB hitbox count
    // assumes both hitbox counts are >= 1
    assert(Tables.EntityHitbox.CollisionOrderTable.SHIFT == 2)
    asl
    asl
    ora.b   entityA.hitboxCount
    asl
    tax

    lda.l   Tables.EntityHitbox.CollisionOrderTable - 2 * 4,x
    tax

    // Assumes first item in CollisionOrder is always valid

    lda.l   Tables.EntityHitbox.CollisionOrder,x

    InnerLoop:
        // A = CollisionOrder row
        // X = collisionOrderPos

        sta.b   _row
        stx.b   _collisonOrderPos

        and.w   #0xff
        clc
        adc.b   entityB.hitboxAddress
        tax

        lda.b   _row + 1
        and.w   #0xff
        clc
        adc.b   entityA.hitboxAddress
        tay

        // Y = entityA hitbox
        // X = entityB hitbox

        _BranchOnInnerInnerCollision(_InnerCollision)

        ldx.b   _collisonOrderPos
        inx
        inx

        lda.l   Tables.EntityHitbox.CollisionOrder,x
        bpl     InnerLoop

    jmp     Continue_EntityLoop


_InnerCollision:
    // Y = entityA hitbox address + inner hitbox offset
    ldx.b   entityA.entityAddress

    // calculate entityB hitboxAddress

    lda.b   _row
    and.w   #0xff
    clc
    adc.b   entityB.hitboxAddress
    sta.b   entityB.hitboxAddress

    jmp     InnerCollision
}
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

