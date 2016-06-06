
scope MetaSprite {

// Initializes the MetaSprite Engine
//
// This MUST be called before using this module.
//
// REQUIRES: 8 bit A, 8 bit Index, DB = $7E
code()
macro Init() {
    assert8a()
    assert8i()

    MetaSprite.Palette._Init()
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
    // X = frameSet
    jsr     MetaSprite.Palette._ProcessSlots
    // c = return

    rep     #$20
a16()
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

    rep     #$30
a16()
a16()
    rts
}


}

// vim: ft=asm ts=4 sw=4 et:

