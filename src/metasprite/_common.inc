
scope MetaSprite {

// Initializes the MetaSprite Engine
//
// This MUST be called before using this module.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
code()
macro Init() {
    assert16a()
    assert16i()

    MetaSprite.Render._Init()

    sep     #$30
a8()
i8()
    MetaSprite.Palette._Init()
    MetaSprite.Vram._Init()

    rep     #$30
a16()
i16()
}



// Initializes a new metasprite entity.
//
// NOTE: This routine does not allocate palette/vram
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
code()
macro _InitEntity() {
    assert16a()
    assert16i()

    scope InitEntity: {
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

    scope EntityDestructor: {
        jsr     MetaSprite.Deactivate
        stz.b   BaseEntity.MetaSprite.frameSet
        stz.b   BaseEntity.MetaSprite.currentFrame
    }
}



// Activates an entity, allocating resources to it
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $7E
//
// INPUT: DP = Entity
// OUTPUT: C set if successful
code()
a16()
i16()
scope Activate: {
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
code()
a16()
i16()
scope Deactivate: {
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
code()
a16()
i16()
scope SetFrame: {
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
code()
a16()
i16()
scope GetFrameAddress: {
    and.w   #0x00ff
    asl
    adc.l   MetaSprite.Format.FrameSet.frameTable,x
    tax
    lda.l   MetaSprite.Data.FrameList,x
    tax
    rts
}

}

// vim: ft=asm ts=4 sw=4 et:

