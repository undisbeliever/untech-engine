// metasprite/collisionboxes.inc
// =============================
//
// MetaSprite Entity Hitbox collision tests.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {
namespace CollisionBoxes {


// PUBLIC API
// ==========


// Start an entity collision box test block.
//
// You MUST ONLY invoke `MetaSprite.CollisionBoxes.TestCollisions`
// inside the CollisionBoxes Begin()-End() block.
//
// You MUST invoke `MetaSprite.CollisionBoxes.End` to end the block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = 0x7e
// OUTPUT: DB = CollisionBoxBank
macro Begin() {
    assert16a()
    assert16i()

    if MetaSprite.CollisionBoxes.__EH_BLOCK != 0 {
        error "Cannot invoke MetaSprite.CollisionBoxes.Begin() twice"
    }
    MetaSprite.CollisionBoxes.__EH_BLOCK = 1

    pea     (0x7e << 8) | MetaSprite.CollisionBoxes.CollisionBoxBank
    plb

    // Ensure the third byte of the offset/size pair is zero
    assert(MetaSprite.CollisionBoxes._xPosWidthPair + 3 == MetaSprite.CollisionBoxes._yPosHeightPair)
    stz.b   MetaSprite.CollisionBoxes._xPosWidthPair + 1
    stz.b   MetaSprite.CollisionBoxes._yPosHeightPair + 1
}



// End the entity collision box test block.
//
// Restores the DP/DB registers.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxesBank
// OUTPUT: DB = 0x7e
macro End() {
    if MetaSprite.CollisionBoxes.__EH_BLOCK == 0 {
        error "Expected MetaSprite.CollisionBoxes.Begin()"
    }
    if MetaSprite.CollisionBoxes.__EH_BLOCK == 1 {
        error "Expected at least one MetaSprite.CollisionBoxes.TestCollisions()"
    }

    MetaSprite.CollisionBoxes.__EH_BLOCK = 0

    plb
}



// Preforms an entity collision test between the player's hurtbox and the shield/hitbox
// of an entity list.
//
// This macro MUST BE invoked between a `MetaSprite.CollisionBoxes.Begin()` and
// `MetaSprite.CollisionBoxes.End()` block.
//
// This macro SHOULD only be called once (inside the game-loop).

// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityList1 : entityListN - entity lists to test for collision with player
macro TestPlayerHitboxAndHurtboxAgainstEntityList(entityList) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
            // Player has a hitbox and a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList})
            bra     EndIf{#}

    NoPlayerHurtbox{#}:
        // Player only has a hitbox
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList})
        bra     EndIf{#}

    NoPlayerHitbox{#}:
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(EndIf{#})
            // Player only has a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList})
EndIf{#}:
}
macro TestPlayerHitboxAndHurtboxAgainstEntityList(entityList1, entityList2) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
            // Player has a hitbox and a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList2})
            bra     EndIf{#}

        NoPlayerHitbox{#}:
            bra     NoPlayerHitbox_Long{#}

    NoPlayerHurtbox{#}:
        // Player only has a hitbox
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
        bra     EndIf{#}

    NoPlayerHitbox_Long{#}:
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(EndIf{#})
            // Player only has a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
EndIf{#}:
}
macro TestPlayerHitboxAndHurtboxAgainstEntityList(entityList1, entityList2, entityList3) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
            // Player has a hitbox and a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList2})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList3})
            bra     EndIf{#}

        NoPlayerHitbox{#}:
            bra     NoPlayerHitbox_Long{#}

    NoPlayerHurtbox{#}:
        // Player only has a hitbox
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList3})
        bra     EndIf{#}

    NoPlayerHitbox_Long{#}:
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(EndIf{#})
            // Player only has a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList3})
EndIf{#}:
}
macro TestPlayerHitboxAndHurtboxAgainstEntityList(entityList1, entityList2, entityList3, entityList4) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
            // Player has a hitbox and a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList2})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList3})
            MetaSprite.CollisionBoxes._ProcessList_PlayerHitboxAndHurtbox_vs_EntityList({entityList4})
            jmp     EndIf{#}

        NoPlayerHitbox{#}:
            bra     NoPlayerHitbox_Long{#}

    NoPlayerHurtbox{#}:
        // Player only has a hitbox
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList3})
        MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList4})
        bra     EndIf{#}

    NoPlayerHitbox_Long{#}:
        MetaSprite.CollisionBoxes._SetupPlayerHurtbox(EndIf{#})
            // Player only has a hurtbox
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList3})
            MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList4})
EndIf{#}:
}



// Preforms an entity collision test between the player's hurtbox and the shield/hitbox
// of an entity list.
//
// This macro MUST BE invoked between a `MetaSprite.CollisionBoxes.Begin()` and
// `MetaSprite.CollisionBoxes.End()` block.
//
// This macro SHOULD only be called once (inside the game-loop).

// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityList1 : entityListN - entity lists to test for collision with player
macro TestPlayerHitboxAgainstEntityList(entityList) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList})
NoPlayerHitbox{#}:
}
macro TestPlayerHitboxAgainstEntityList(entityList1, entityList2) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
NoPlayerHitbox{#}:
}
macro TestPlayerHitboxAgainstEntityList(entityList1, entityList2, entityList3) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList3})
NoPlayerHitbox{#}:
}
macro TestPlayerHitboxAgainstEntityList(entityList1, entityList2, entityList3, entityList4) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHitbox(NoPlayerHitbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList2})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList3})
    MetaSprite.CollisionBoxes._ProcessList_Hitbox_vs_ShieldAndHurtbox({entityList4})
NoPlayerHitbox{#}:
}



// Preforms an entity collision test between the player's hitbox and the shield/hurtbox
// of an entity list.
//
// This macro MUST BE invoked between a `MetaSprite.CollisionBoxes.Begin()` and
// `MetaSprite.CollisionBoxes.End()` block.
//
// This macro SHOULD only be called once (inside the game-loop).
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityList1 : entityListN - entity lists to test for collision with player
macro TestPlayerHurtboxAgainstEntityList(entityList) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList})
NoHurtbox{#}:
}
macro TestPlayerHurtboxAgainstEntityList(entityList1, entityList2) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
NoPlayerHurtbox{#}:
}
macro TestPlayerHurtboxAgainstEntityList(entityList1, entityList2, entityList3) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList3})
NoPlayerHurtbox{#}:
}
macro TestPlayerHurtboxAgainstEntityList(entityList1, entityList2, entityList3, entityList4) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._SetupPlayerHurtbox(NoPlayerHurtbox{#})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList1})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList2})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList3})
    MetaSprite.CollisionBoxes._ProcessList_Hurtbox_vs_ShieldAndHitbox({entityList4})
NoPlayerHurtbox{#}:
}



// Preforms an entity collision test between the hitbox of the entities in
// an entity list and the shield/hurtbox of the entity list.
//
// This macro MUST BE invoked between a `MetaSprite.CollisionBoxes.Begin()` and
// `MetaSprite.CollisionBoxes.End()` block.
//
// This macro SHOULD only be called once (inside the game-loop).
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: hitboxEntityList : entity list to test hitboxes
// PARAM: hurtboxEntityList : entity list to test shield/hurtbox
macro TestEntityListHitboxAgainstEntityList(hitboxEntityList, hurtboxEntityList) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_Start({hitboxEntityList}, {#})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_End({#})
}
macro TestEntityListHitboxAgainstEntityList(hitboxEntityList, hurtboxEntityList1, hurtboxEntityList2) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_Start({hitboxEntityList}, {#})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList1})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList2})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_End({#})
}
macro TestEntityListHitboxAgainstEntityList(hitboxEntityList, hurtboxEntityList1, hurtboxEntityList2, hitboxEntityList3) {
    assert16a()
    assert16i()

    MetaSprite.CollisionBoxes._TestInBlock()

    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_Start({hitboxEntityList}, {#})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList1})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList2})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_TestList({hurtboxEntityList3})
    MetaSprite.CollisionBoxes._ListHitboxVsListHurtbox_End({#})
}



// VARIABLES
// =========


// CollisionBoxes Begin()-End() block status
variable __EH_BLOCK = 0

// Data bank containing the Collision Box data
constant CollisionBoxBank = Data.CollisionBoxData >> 16


// Temporary store for the offset/size pair from CollisionBoxes struct
// Used to convert width/height from uint8 to uint16
// Also used to store the last hitbox read by the test collision code
// (2x 3x uint8_t, third byte always 0)
allocate(_xPosWidthPair, dp, 3)
allocate(_yPosHeightPair, dp, 3)

// Test assumption made by _xPosWidthPair/_yPosHeightPair optimisation
assert(Format.CollisionBoxes.Shield.xOffset + 1 == Format.CollisionBoxes.Shield.width)
assert(Format.CollisionBoxes.Shield.yOffset + 1 == Format.CollisionBoxes.Shield.height)
assert(Format.CollisionBoxes.Hurtbox.xOffset + 1 == Format.CollisionBoxes.Hurtbox.width)
assert(Format.CollisionBoxes.Hurtbox.yOffset + 1 == Format.CollisionBoxes.Hurtbox.height)
assert(Format.CollisionBoxes.Hitbox.xOffset + 1 == Format.CollisionBoxes.Hitbox.width)
assert(Format.CollisionBoxes.Hitbox.yOffset + 1 == Format.CollisionBoxes.Hitbox.height)


// The outer loop entity variables
namespace entityA {
    // address of the entity
    // (word addr)
    allocate(entityAddress, dp, 2)

    // The hurtbox to test against.
    // all values are offset by INT_MS8_OFFSET
    // (4x uint16)
    namespace hurtboxAabb {
        allocate(left, dp, 2)
        allocate(right, dp, 2)
        allocate(top, dp, 2)
        allocate(bottom, dp, 2)
    }

    // The hitbox to test against.
    // all values are offset by INT_MS8_OFFSET
    // (4x uint16)
    namespace hitboxAabb {
        allocate(left, dp, 2)
        allocate(right, dp, 2)
        allocate(top, dp, 2)
        allocate(bottom, dp, 2)
    }
}


// The inner loop entity variables
namespace entityB {
    // entityAddress must survive a ProcessCollision function call
    // (word addr)
    allocate(entityAddress, dp, 2)
}

// Temporary Variables

constant _tmp                   = utDpTmp0



// PRIVATE API
// ===========


// Tests that we are in a CollisionBoxes Begin()-End() block
inline _TestInBlock() {
    if MetaSprite.CollisionBoxes.__EH_BLOCK == 0 {
        error "Expected MetaSprite.CollisionBoxes.Begin()"
    }
    MetaSprite.CollisionBoxes.__EH_BLOCK = MetaSprite.CollisionBoxes.__EH_BLOCK + 1
}



// Calculates the aabb of an entity's box.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
// PARAM: box = the name of the box to access
// PARAM: aabb = the location to store the aabb.
// PARAM: NoBoxLabel - label to branch to if `box` is empty
// INPUT: X = entity
//        Y = CollisionBoxes address
// OUTPUT: _xPosWidthPair/_yPosHeightPair = the outer hitbox data
// BRANCH: NoBoxLabel if there the `box` is empty
macro _SetupAabb(box, aabb, NoBoxLabel) {
    assert16a()
    assert16i()


    lda.w   MetaSprite.Format.CollisionBoxes.{box}.xOffset,y
    bmi     {NoBoxLabel}

    sta.b   MetaSprite.CollisionBoxes._xPosWidthPair

    and.w   #0xff
    clc
    adc.b   BaseEntity.xPos.px,x
    sta.b   MetaSprite.CollisionBoxes.{aabb}.left

    sec                                                     // + 1
    adc.b   MetaSprite.CollisionBoxes._xPosWidthPair + 1    // width
    sta.b   MetaSprite.CollisionBoxes.{aabb}.right


    lda.w   MetaSprite.Format.CollisionBoxes.{box}.yOffset,y
    sta.b   MetaSprite.CollisionBoxes._yPosHeightPair

    and.w   #0xff
    clc
    adc.b   BaseEntity.yPos.px,x
    sta.b   MetaSprite.CollisionBoxes.{aabb}.top

    sec                                                     // + 1
    adc.b   MetaSprite.CollisionBoxes._yPosHeightPair + 1   // height
    sta.b   MetaSprite.CollisionBoxes.{aabb}.bottom
}



// Tests for a collision between an entity's outer hitbox and a given aabb,
// branching to a given label if a collision occurred.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
// PARAM: box = the name of the box to access
// PARAM: aabb = the aabb to compare against
// INPUT: X = entity
//        Y = CollisionBoxes address
// KEEP: X, Y
// OUTPUT: _xPosWidthPair/_yPosHeightPair = the box data (if a collision occurred)
// BRANCH: to `CollisionLabel` if the box exists and there is a collision
macro _BranchIfCollision(aabb, box, CollisionLabel) {
    assert16a()
    assert16i()

    // x coordinate more likely to miss

    // if right > 128: goto NoCollision
    //
    // if left >= aabb.right
    //    || right <= aabb.left: goto NoCollision

    lda.w   Format.CollisionBoxes.{box}.xOffset,y
    bmi     NoCollision_{#}

    sta.b   _xPosWidthPair
    and.w   #0xff
    clc
    adc.b   BaseEntity.xPos.px,x

    cmp.b   {aabb}.right
    bcs     NoCollision_{#}

    // c clear
    adc.b   _xPosWidthPair + 1      // width
    // A = right - 1
    cmp.b   {aabb}.left
    bcc     NoCollision_{#}


    lda.w   Format.CollisionBoxes.{box}.yOffset,y
    sta.b   _yPosHeightPair
    and.w   #0xff
    clc
    adc.b   BaseEntity.yPos.px,x

    cmp.b   {aabb}.bottom
    bcs     NoCollision_{#}

    // c clear
    adc.b   _yPosHeightPair + 1     // height
    // A = bottom - 1
    cmp.b   {aabb}.top
    bcs     {CollisionLabel}

NoCollision_{#}:
}



// Populate `entityA.hitboxAabb` with the player's hitbox.
//
// If the MetaSprite frame has no hitbox, then this macro will jump to `NoHitboxLabel`.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// BRANCH: to `NoHitboxLabel` if the player does not have a hitbox
inline _SetupPlayerHitbox(NoHitboxLabel) {
    ldy.w   #Entity.playerEntity
    sty.b   MetaSprite.CollisionBoxes.entityA.entityAddress

    // ::TODO add collisionBox index to BaseEntity::
    ldx.b   BaseEntity.MetaSprite.currentFrame,y
    lda.l   MetaSprite.Format.Frame.collisionBoxes,x
    tyx
    tay

    MetaSprite.CollisionBoxes._SetupAabb(Hitbox, entityA.hitboxAabb, {NoHitboxLabel})
}



// Populate `entityA.hurtboxAabb` with the player's hurtbox.
//
// If the MetaSprite frame has no hurtbox, then this macro will jump to `NoHitboxLabel`.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// BRANCH: to `NoHurtboxLabel` if the player does not have a hurtbox
inline _SetupPlayerHurtbox(NoHurtboxLabel) {
    ldy.w   #Entity.playerEntity
    sty.b   MetaSprite.CollisionBoxes.entityA.entityAddress

    // ::TODO add collisionBox index to BaseEntity::
    ldx.b   BaseEntity.MetaSprite.currentFrame,y
    lda.l   MetaSprite.Format.Frame.collisionBoxes,x
    tyx
    tay

    MetaSprite.CollisionBoxes._SetupAabb(Hurtbox, entityA.hurtboxAabb, {NoHurtboxLabel})
}



// Tests for a collisions between a single entity's hitbox and
// the shield/hurtbox of each entity in an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
// PARAM: entityList - the name of the entity list
// INPUT: entityA.hitboxAabb = the aabb of the hitbox to compare against
// INPUT: entityA.entityAddress = the address of the entity to compare against
inline _ProcessList_Hitbox_vs_ShieldAndHurtbox(entityList) {
    assert16a()
    assert16i()

    lda.l   {Entity.listAddress({entityList})}
    beq     EndIf{#}
        tay
        jsr     MetaSprite.CollisionBoxes.__ProcessList_Hitbox_vs_ShieldAndHurtbox
    EndIf{#}:
}



// Tests for collisions between a single entity's hurtbox and
// the sheild/hitbox of each entity in an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
// PARAM: entityList - the name of the entity list
// INPUT: entityA.hurtboxAabb = the aabb of the hurtbox to compare against
// INPUT: entityA.entityAddress = the address of the entity to compare against
inline _ProcessList_Hurtbox_vs_ShieldAndHitbox(entityList) {
    assert16a()
    assert16i()

    lda.l   {Entity.listAddress({entityList})}
    beq     EndIf{#}
        tay
        jsr     MetaSprite.CollisionBoxes.__ProcessList_Hurtbox_vs_ShieldAndHitbox
    EndIf{#}:
}



// Tests for a collisions between the player's hitbox and hurtbox and
// the collision boxes of each entity in an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
// PARAM: entityList - the name of the entity list
// INPUT: entityA.hitboxAabb = the aabb of the hitbox to compare against
// INPUT: entityA.hurtboxAabb = the aabb of the hurtbox to compare against
inline _ProcessList_PlayerHitboxAndHurtbox_vs_EntityList(entityList) {
    assert16a()
    assert16i()

    lda.l   {Entity.listAddress({entityList})}
    beq     EndIf{#}
        tay
        jsr     MetaSprite.CollisionBoxes.__ProcessList_PlayerHitboxAndHurtbox_vs_EntityList
    EndIf{#}:
}



// Start a TestEntityListHitboxAgainstEntityList block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityList - the name of the entity list
// PARAM: labelSuffix - the suffix to apply to each label in the macro
inline _ListHitboxVsListHurtbox_Start(entityList, labelSuffix) {
    assert16a()
    assert16i()

    lda.l   {Entity.listAddress({entityList})}
    beq     ListEmpty_{labelSuffix}

    tay

    Loop_{labelSuffix}:
        // Y = entity address (not null)
        sty.b   MetaSprite.CollisionBoxes.entityA.entityAddress

        // ::TODO add collisionBox index to BaseEntity::
        ldx.b   BaseEntity.MetaSprite.currentFrame,y
        lda.l   MetaSprite.Format.Frame.collisionBoxes,x
        tyx
        tay

        MetaSprite.CollisionBoxes._SetupAabb(Hitbox, entityA.hitboxAabb, NoHurtboxLabel_{labelSuffix})
}


// Test for a collision between entityA's hitbox and
// the shield/hurtbox of each entity in the entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityList - the name of the entity list
//
// INPUT: entityA.hitboxAabb = the aabb of the hitbox to compare against
// INPUT: entityA.entityAddress = the address of the entity to compare against
inline _ListHitboxVsListHurtbox_TestList(entityList) {
    assert16a()
    assert16i()

        lda.l   {Entity.listAddress({entityList})}
        beq     EndIf{#}
            tay
            jsr     MetaSprite.CollisionBoxes.__ProcessList_Hitbox_vs_ShieldAndHurtbox
        EndIf{#}:
}


// End a TestEntityListHitboxAgainstEntityList block.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: labelSuffix - the suffix that was applied to the _Start macro
inline _ListHitboxVsListHurtbox_End(labelSuffix) {
    assert16a()
    assert16i()

    NoHurtboxLabel_{labelSuffix}:
        ldx.w   MetaSprite.CollisionBoxes.entityA.entityAddress
        ldy.b   BaseEntity.next,x
        bne     Loop_{labelSuffix}

    ListEmpty_{labelSuffix}:
}



// Calculates the centre of the collision for a single axis.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
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



// Calculates the centre of a collision with entityA.hurtBox, storing it in `MetaSprite.Collision`.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// INPUT: entityA.hurtBox = hitbox used in last test
// INPUT: X = entity
// INPUT: _xPosWidthPair/_yPosHeightPair = entity hitbox data of last test
// KEEP: X/Y
// OUTPUT: MetaSprite.Collision.xPos, MetaSprite.Collision.yPos
a16()
i16()
code()
function _CalculateCollisionCentre_Hurtbox {
    __CalculateCollisionCentreAxis(xPos, _xPosWidthPair, entityA.hurtboxAabb.left, entityA.hurtboxAabb.right)
    __CalculateCollisionCentreAxis(yPos, _yPosHeightPair, entityA.hurtboxAabb.top, entityA.hurtboxAabb.bottom)

    rts
}



// Calculates the centre of a collision with entityA.hitBox, storing it in `MetaSprite.Collision`.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// INPUT: entityA.hitBox = hitbox used in last test
// INPUT: X = entity
// INPUT: _xPosWidthPair/_yPosHeightPair = entity hitbox data of last test
// KEEP: X/Y
// OUTPUT: MetaSprite.Collision.xPos, MetaSprite.Collision.yPos
a16()
i16()
code()
function _CalculateCollisionCentre_Hitbox {
    __CalculateCollisionCentreAxis(xPos, _xPosWidthPair, entityA.hitboxAabb.left, entityA.hitboxAabb.right)
    __CalculateCollisionCentreAxis(yPos, _yPosHeightPair, entityA.hitboxAabb.top, entityA.hitboxAabb.bottom)

    rts
}



// Calls the entity collision functions.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityBfunction - the collision function to call for entityB (will be called first)
// PARAM: entityBCarrySet - state of the carry flag when `entityBfunction` is called (non-zero = carry set)
// PARAM: entityAfunction - the collision function to call for entityA (will be called second)
// PARAM: entityACarrySet - state of the carry flag when `entityAfunction` is called (non-zero = carry set)
//
// INPUT: X = entityB address
// INPUT: entityA.entityAddress = entityA address
// OUTPUT: entityB.entityAddress = entityB address
// MODIFES: X
inline __CallCollisionCallbacks(entityBfunction, evaluate entityBCarrySet, entityAfunction, evaluate entityACarrySet) {
    assert16a()
    assert16i()

    stx.b   entityB.entityAddress

    pea     (CollisionBoxBank << 8) | 0x7e
    plb
// DB = 0x7e


    ldy.w   entityA.entityAddress
    txa
    tcd
// DP = entityB

    if {entityBCarrySet} {
        sec
    } else {
        clc
    }
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.{entityBfunction},x)


    lda.w   entityA.entityAddress
    tcd
// DP = entityA
    ldy.w   entityB.entityAddress

    if {entityACarrySet} {
        sec
    } else {
        clc
    }
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.{entityAfunction},x)


    lda.w   #0
    tcd
// DP = 0

    plb
// DB = CollisionBoxBank
}



// Calls the entity collision functions.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// PARAM: entityBfunction - the collision function to call for entityB (will be called first)
// PARAM: entityBCarrySet - state of the carry flag when `entityBfunction` is called (non-zero = carry set)
// PARAM: entityAfunction - the collision function to call for entityA (will be called second)
// PARAM: entityACarrySet - state of the carry flag when `entityAfunction` is called (non-zero = carry set)
//
// INPUT: X = entityB address
// OUTPUT: entityB.entityAddress = entityB address
// MODIFES: X
inline __CallCollisionCallbacks_Player(entityBfunction, evaluate entityBCarrySet, entityAfunction, evaluate entityACarrySet) {
    assert16a()
    assert16i()

    stx.b   entityB.entityAddress

    pea     (CollisionBoxBank << 8) | 0x7e
    plb
// DB = 0x7e


    ldy.w   #Entity.playerEntity
    txa
    tcd
// DP = entityB

    if {entityBCarrySet} {
        sec
    } else {
        clc
    }
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.{entityBfunction},x)


    lda.w   #Entity.playerEntity
    tcd
// DP = playerEntity
    ldy.w   entityB.entityAddress

    if {entityACarrySet} {
        sec
    } else {
        clc
    }
    ldx.b   BaseEntity.functionTable
    jsr     (BaseEntityFunctionTable.{entityAfunction},x)


    lda.w   #0
    tcd
// DP = 0

    plb
// DB = CollisionBoxBank
}



namespace __ProcessList_Hitbox_vs_ShieldAndHurtbox {

ShieldCollision:
    jsr     _CalculateCollisionCentre_Hitbox

    __CallCollisionCallbacks(ShieldCollision, 1, HitboxCollision, 1)


ContinueLoopAfterCollision:
    ldx.b   entityB.entityAddress
    ldy.b   BaseEntity.next,x
    beq     Return

Fallthrough:
}


// Tests for a collisions between a single entity's hitbox and
// the shield/hurtbox of each entity in an entity list.
//
// If there is a collision, the appropriate collision functions for both
// entities will be called.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// INPUT: Y = first item in listB (NOT NULL)
// INPUT: entityA.hitboxAabb = the aabb of the hitbox to compare against
// INPUT: entityA.entityAddress = the address of the entity to compare against
a16()
i16()
code()
function __ProcessList_Hitbox_vs_ShieldAndHurtbox {
    assert(pc() == Fallthrough)

    Loop:
        // Y = entity address

        // ::TODO add collisionBoxes index to BaseEntity::
        ldx.b   BaseEntity.MetaSprite.currentFrame,y

        if Data.FrameList >> 16 == CollisionBoxBank {
            lda.w   Format.Frame.collisionBoxes,x
        } else {
            lda.l   Format.Frame.collisionBoxes,x
        }
        tyx
        tay

        // X = entity address
        // Y = hitbox address

        _BranchIfCollision(entityA.hitboxAabb, Shield, ShieldCollision)
        _BranchIfCollision(entityA.hitboxAabb, Hurtbox, HurtboxCollision)

    ContinueLoop:
        // X = entity address
        ldy.b   BaseEntity.next,x
        bne     Loop

Return:
    rts


HurtboxCollision:
    jsr     _CalculateCollisionCentre_Hitbox

    __CallCollisionCallbacks(HurtboxCollision, 0, HitboxCollision, 0)

    jmp     ContinueLoopAfterCollision
}




namespace __ProcessList_Hurtbox_vs_ShieldAndHitbox {

ShieldCollision:
    jsr     _CalculateCollisionCentre_Hurtbox

    __CallCollisionCallbacks(ShieldCollision, 0, HurtboxCollision, 1)

ContinueLoopAfterCollision:
    ldx.b   entityB.entityAddress
    ldy.b   BaseEntity.next,x
    beq     Return

Fallthrough:
}


// Tests for collisions between a single entity's hurtbox and
// the sheild/hitbox of each entity in an entity list.
//
// If there is a collision, the appropriate collision functions for both
// entities will be called.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// INPUT: Y = first item in listB (NOT NULL)
// INPUT: entityA.hurtboxAabb = the aabb of the hurtbox to compare against
// INPUT: entityA.entityAddress = the address of the entity to compare against
a16()
i16()
code()
function __ProcessList_Hurtbox_vs_ShieldAndHitbox {
    assert(pc() == Fallthrough)

    Loop:
        // Y = entity address

        // ::TODO add collisionBoxes index to BaseEntity::
        ldx.b   BaseEntity.MetaSprite.currentFrame,y

        if Data.FrameList >> 16 == CollisionBoxBank {
            lda.w   Format.Frame.collisionBoxes,x
        } else {
            lda.l   Format.Frame.collisionBoxes,x
        }
        tyx
        tay

        // X = entity address
        // Y = hitbox address

        _BranchIfCollision(entityA.hurtboxAabb, Shield, ShieldCollision)
        _BranchIfCollision(entityA.hurtboxAabb, Hitbox, HitboxCollision)

    ContinueLoop:
        // X = entity address
        ldy.b   BaseEntity.next,x
        bne     Loop

Return:
    rts


HitboxCollision:
    jsr     _CalculateCollisionCentre_Hurtbox

    __CallCollisionCallbacks(HitboxCollision, 0, HurtboxCollision, 0)

    jmp     ContinueLoopAfterCollision
}



namespace __ProcessList_PlayerHitboxAndHurtbox_vs_EntityList {

Return:
        rts


HitboxShieldCollision:
    jsr     _CalculateCollisionCentre_Hitbox

    __CallCollisionCallbacks_Player(ShieldCollision, 1, HitboxCollision, 1)


ContinueLoopAfterCollision:
    ldx.b   entityB.entityAddress
    ldy.b   BaseEntity.next,x
    beq     Return

Fallthrough:
}


// Tests for a collisions between the player's hitbox and hurtbox and
// the collision boxes of each entity in an entity list.
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0, DB = CollisionBoxBank
//
// INPUT: Y = first item in listB (NOT NULL)
// INPUT: entityA.hittboxAabb = the aabb of the hitbox to compare against
// INPUT: entityA.hurtboxAabb = the aabb of the hurtbox to compare against
a16()
i16()
code()
function __ProcessList_PlayerHitboxAndHurtbox_vs_EntityList {
    assert(pc() == Fallthrough)

    Loop:
        // Y = entity address

        // ::TODO add collisionBoxes index to BaseEntity::
        ldx.b   BaseEntity.MetaSprite.currentFrame,y

        if Data.FrameList >> 16 == CollisionBoxBank {
            lda.w   Format.Frame.collisionBoxes,x
        } else {
            lda.l   Format.Frame.collisionBoxes,x
        }
        tyx
        tay

        // X = entity address
        // Y = hitbox address

        _BranchIfCollision(entityA.hitboxAabb, Shield, HitboxShieldCollision)
        _BranchIfCollision(entityA.hurtboxAabb, Shield, HurtboxShieldCollision)
        _BranchIfCollision(entityA.hitboxAabb, Hurtbox, HurtboxCollision)
        _BranchIfCollision(entityA.hurtboxAabb, Hitbox, HitboxCollision)

    ContinueLoop:
        // X = entity address
        ldy.b   BaseEntity.next,x
        beq     +
            jmp     Loop
        +

    rts



HurtboxShieldCollision:
    jsr     _CalculateCollisionCentre_Hurtbox

    __CallCollisionCallbacks_Player(ShieldCollision, 0, HurtboxCollision, 1)

    jmp     ContinueLoopAfterCollision


HurtboxCollision:
    jsr     _CalculateCollisionCentre_Hitbox

    __CallCollisionCallbacks_Player(HurtboxCollision, 0, HitboxCollision, 0)

    jmp     ContinueLoopAfterCollision


HitboxCollision:
    jsr     _CalculateCollisionCentre_Hurtbox

    __CallCollisionCallbacks_Player(HitboxCollision, 0, HurtboxCollision, 0)

    jmp     ContinueLoopAfterCollision
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

