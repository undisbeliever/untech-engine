
scope MetaSpriteTest {

// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
code()
a16()
i16()
scope _Init_MetaSpriteModule: {
    jsr     Dma.Init

    MetaSprite.Init()

    stz.w   Dma.Palette.dirty

    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
code()
a16()
i16()
scope _InitEntity: {
    MetaSprite._InitEntity()
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
// INPUT:  Y = palette Id
code()
a16()
i16()
scope _InitEntityAndActivate: {
    jsr     _InitEntity
    jmp     MetaSprite.Activate
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
code()
a16()
i16()
scope _InitEntity0Palette: {
    ldy.w   #0
    jmp     _InitEntity
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  A = frameSet Id
code()
a16()
i16()
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
code()
a16()
i16()
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

// vim: ft=asm ts=4 sw=4 et:

