
scope MetaSpriteTest {

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
a16()
i16()
code()
scope _Init_MetaSpriteModule: {
    jsr     Dma.Init
    jsr     MetaSprite.Init

    stz.w   Dma.Palette.dirty

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
a16()
i16()
code()
scope _InitEntity: {
    MetaSprite._InitEntity()
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
a16()
i16()
code()
scope _InitEntityAndActivate: {
    jsr     _InitEntity
    jmp     MetaSprite.Activate
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
a16()
i16()
code()
scope _InitEntity0Palette: {
    ldy.w   #0
    jmp     _InitEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
a16()
i16()
code()
scope _InitEntity0PaletteAndActivate: {
    ldy.w   #0
    jmp     _InitEntityAndActivate
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
// INPUT:  X = frame Id
// OUTPUT: C set on success
a16()
i16()
code()
scope _InitEntityFrameActivate: {
    phx
        jsr     _InitEntity
    pla

    jsr     MetaSprite.SetFrame
    bcc     Fail

    jmp     MetaSprite.Activate

Fail:
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

