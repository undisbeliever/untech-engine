// metasprite/_common.inc
// ======================
//
// Common routines for the MetaSprite Module.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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

    function InitEntity {
        stz.b   BaseEntity.MetaSprite.status
        stz.b   BaseEntity.MetaSprite.currentFrame

        // Determine FrameSet address
        cmp.w   #MetaSprite.Data.FrameSetListCount
        bcc     +
            lda.w   #0
        +

        asl
        tax
        lda.l   MetaSprite.Data.FrameSetList,x
        sta.b   BaseEntity.MetaSprite.frameSet
        tax

        sep     #$10
    i8()
        sty.b   BaseEntity.MetaSprite.palette

        // Disable animation
        ldx.b   #0xff
        stx.b   BaseEntity.MetaSprite.Animation.id

        rep     #$30
    a16()
    i16()
    }
}



// Destructor for the metasprite entity
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
macro EntityDestructor() {
    assert16a()
    assert16i()

    function EntityDestructor {
        jsr     MetaSprite.Deactivate
        stz.b   BaseEntity.MetaSprite.frameSet
        stz.b   BaseEntity.MetaSprite.currentFrame

        // Disable animation
        ldx.w   #0xffff
        stx.b   BaseEntity.MetaSprite.Animation.id
    }
}



// Activates an entity, allocating resources to it
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: DP = Entity
// OUTPUT: C set if successful
a16()
i16()
code()
function Activate {
    ldx.b   BaseEntity.MetaSprite.frameSet
    beq     Return

    sep     #$20
a8()
    Vram._Activate()

    sep     #$20
    rep     #$10
a8()
i16()

    ldx.b   BaseEntity.MetaSprite.frameSet
    jsr     MetaSprite.Palette._ProcessSlots
    // c = return

ReturnRep:
    rep     #$30
a16()
i16()
Return:
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


// Sets the entity's animation.
//
// Will not reset the animation if the animation Id is unchanged.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
// INPUT: A = animation Id
// OUTPUT: C set on success
constant SetAnimation = Animation.SetAnimationId


// Disables the entity's animation.
//
// REQUIRES: 16 bit A, 16 bit Index
//
// INPUT: dp = Entity
constant DisableAnimation = Animation.DisableAnimation


// Sets the entity's metasprite frame.
//
// If the frame is active and has a dynamic tileset then a new tileset
// is loaded into vram.
//
// This routine will fail IF:
//      * entity has not FrameSet
//      * frameId is invalid
//      * FrameSet uses a dynamic tileset and there is not enough dma
//        time to upload the tileset in the next VBlank.
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
    beq     ReturnFalse

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

        sta.b   BaseEntity.MetaSprite.currentFrame

        sec
        rts

ReturnFalse:
    rep     #$31
    rts


a16()
DynamicTileset:
        // Activated with a Dynamic Tileset
        and.w   #0x00ff
        asl
        adc.l   MetaSprite.Format.FrameSet.frameTable,x
        tax
        lda.l   MetaSprite.Data.FrameList,x

        pha
        tax
            jsr     Vram._UploadDynamicTileset.GivenFrame
        pla
        bcc     Return

        sta.b   BaseEntity.MetaSprite.currentFrame

Return:
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

