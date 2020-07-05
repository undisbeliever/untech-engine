// metasprite/_common.inc
// ======================
//
// Common routines for the MetaSprite Module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace MetaSprite {

// Initializes the MetaSprite Engine
//
// This MUST be called before using this module.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
a16()
i16()
code()
function Init {
    MetaSprite.Render._Init()

    sep     #$30
a8()
i8()
    MetaSprite.Palette._Init()
    MetaSprite.Vram._Init()
    MetaSprite.Animation._Init()

    rep     #$30
a16()
i16()
    rts
}



// Initializes a new metasprite entity.
//
// NOTE: This routine does not allocate palette/vram
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
a16()
i16()
code()
macro _InitEntity() {
    assert16a()
    assert16i()

    stz.b   BaseEntity.MetaSprite.status

    // Determine FrameSet index
    cmp.w   #MetaSprite.Data.FrameSetListCount
    bcc     +
        lda.w   #0
    +

    // frameSet = frameSetId * MetaSprite.Format.FrameSet.size
    assert(MetaSprite.Format.FrameSet.size == 12)
    assert(MetaSprite.Data.FrameSetListCount < (0xffff >> 3))
    asl
    asl
    sta.b   BaseEntity.MetaSprite.frameSet
    asl
    adc.b   BaseEntity.MetaSprite.frameSet
    sta.b   BaseEntity.MetaSprite.frameSet

    // By setting currentFrame to 0 we have a consistent and obvious
    // failure mode for any entity that does not call SetFrame or
    // SetAnimation before the entity is rendered.
    stz.b   BaseEntity.MetaSprite.currentFrame

    sep     #$10
i8()
    sty.b   BaseEntity.MetaSprite.palette

    // Clear OBJ attribute bytes
    stz.b   BaseEntity.MetaSprite.blockOneCharAttrOffset + 1
    stz.b   BaseEntity.MetaSprite.blockTwoCharAttrOffset + 1

    // Disable animation
    ldx.b   #0xff
    stx.b   BaseEntity.MetaSprite.Animation.id

    rep     #$30
a16()
i16()
}



// Destructor for the metasprite entity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
macro EntityDestructor() {
    assert16a()
    assert16i()

    jsr     MetaSprite.Deactivate

    // Disable animation
    ldx.w   #0xffff
    stx.b   BaseEntity.MetaSprite.Animation.id
}



// Activates an entity, allocating resources to it.
//
// NOTE: This routine will return true if the VRAM was activated
//       but the palette was not.
//       When the engine runs out of palettes, the newly activated
//       MetaSprite will have a glitched palette.
//
// NOTE: If the entity uses a dynamic tileset this routine will always try to
//       upload the dynamic tiles to VRAM.
//
// NOTE: FrameSets with a dynamic tileset can fail if the static tiles were
//       uploaded to VRAM but the system ran out of time/space to upload the
//       static tiles.
//
// ASSUMES: The entity has not been activated
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: DP = Entity
// OUTPUT: C set if there is no frameSet or activating VRAM is successful
a16()
i16()
code()
function Activate {
    ldx.b   BaseEntity.MetaSprite.frameSet

    sep     #$20
a8()
    Vram._Activate()
au()
iu()

    sep     #$20
    rep     #$10
a8()
i16()

    ldx.b   BaseEntity.MetaSprite.frameSet
    jsr     MetaSprite.Palette._ProcessSlots

    rep     #$30
a16()
i16()
Return:
    sec
    rts

ReturnFailure:
au()
iu()
    rep     #$31
a16()
i16()
    // carry clear
    rts
}


// Deactivates matasprite, removing its resources but keeping its state.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: DP = Entity
a16()
i16()
code()
function Deactivate {
    sep     #$30
a8()
i8()
    Palette._Deallocate()
    Vram._Deallocate()

    rep     #$30
a16()
a16()
    rts
}


// Sets the entity's metasprite frame.
//
// If the frame is active and has a dynamic tileset then a new tileset
// is loaded into vram.
//
// If the MetaSprite Frame was changed successfully then all the action points
// within the frame will be executed.
//
// This routine will fail IF:
//      * entity has not FrameSet
//      * frameId is invalid
//      * FrameSet uses a dynamic tileset and there is not enough dma
//        time to upload the tileset in the next VBlank.
//
// NOTE: This routine may invalidate `MetaSprite.ActionPoint.positionPair`.
//       DO NOT access `positionPair` after calling this function.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7e
//
// INPUT: DP = Entity
// INPUT: A = frame Id
// OUTPUT: C set if successful
a16()
i16()
code()
function SetFrame {
    ldx.b   BaseEntity.MetaSprite.frameSet

    sep     #$20
a8()
    cmp.l   MetaSprite.Format.FrameSet.nFrames,x
    bcs     ReturnFalse

    assert(StatusFlags.dynamicTileset == 0x40)
    bit.b   BaseEntity.MetaSprite.status
    rep     #$30
a16()
    bvs     DynamicTileset

        // Fixed or Unallocated Tileset
        and.w   #0x00ff
        asl
        adc.l   MetaSprite.Format.FrameSet.frameTable,x
        tax
        lda.l   MetaSprite.Data.FrameList,x
        tax

        stx.b   BaseEntity.MetaSprite.currentFrame

        bra     ProcessActionPoints

ReturnFalse:
    rep     #$31
Return:
    rts


a16()
DynamicTileset:
        // Activated with a Dynamic Tileset
        and.w   #0x00ff
        asl
        adc.l   MetaSprite.Format.FrameSet.frameTable,x
        tax
        lda.l   MetaSprite.Data.FrameList,x
        tax

        phx
            jsr     Vram._UploadDynamicTileset.GivenFrame
        plx
        bcc     Return

        stx.b   BaseEntity.MetaSprite.currentFrame


ProcessActionPoints:
    // Process all action point functions
    // X = frame index

    lda.l   MetaSprite.Format.Frame.actionPoints,x
    beq     NoActionPoints
    tax

    lda.l   MetaSprite.Format.ActionPoints.type,x
    and.w   #Project.ActionPoints.MASK

    Loop:
        tay
        lda.l   MetaSprite.Format.ActionPoints.positionPair,x
        phx
            tyx
            jsr     (Project.ActionPoints,x)
        plx
        inx
        inx
        inx

        lda.l   MetaSprite.Format.ActionPoints.type,x
        and.w   #Project.ActionPoints.MASK
        bne     Loop

NoActionPoints:
    sec
    rts
}


// Retrieves the frame address of a metasprite.
//
// REQUITES: 16 bit A, 16 bit Index, DB = $7e
//
// NOTE: no overflow checking is preformed here
//
// ASSUMES: Entity has a frameSet
//
// INPUT: DP = entity
//        A = frameId
// OUTPUT: X = frameAddress
a16()
i16()
code()
function GetFrameAddress {
    and.w   #0x00ff
    asl
    adc.l   MetaSprite.Format.FrameSet.frameTable,x
    tax
    lda.l   MetaSprite.Data.FrameList,x
    tax
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

