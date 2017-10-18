// resources/_common.inc
// =====================
//
// Resource Management routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

// Initializes the Resources module
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function Init {
    AnimatedTileset.DisableAnimation()
    Palettes.DisableAnimation()

    // Set animation speed

    lda.l   STAT77
    and.w   #STAT78.pal << 8
    beq     +
        // pal
        lda.w   #ANIMATION_TICKS_PER_SECOND / 50
        bra     ++
    +
        // ntsc
        lda.w   #ANIMATION_TICKS_PER_SECOND / 60
    +
    sta.w   animationSpeed

    rts
}


// Process one display from of the any active resource animations.
//
// MUST be called once per frame
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function ProcessAnimations {
    AnimatedTileset._ProcessAnimation()
    Palettes._ProcessAnimation()

    rts
}



// Decompress a lz4 block to VRAM at a given word address
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, force blank
// DMA: Uses DMA channel 0
// MODIFIES: decompressionBuffer
//
// INPUT: LZ4.ptr = address of the lz4 block
//              X = vram word address to load tiles to
a16()
i16()
code()
function DecompressToVram_FromPtr {
    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB == REGISTER_DB

    sep     #$20
a8()
i16()

    stx.w   VMADD


    // Decompress tiles

    ldx.w   #decompressionBuffer
    lda.b   #decompressionBuffer >> 16
    ldy.w   #decompressionBuffer.size

    jsr     LZ4.Decompress


    // Transfer tiles to VRAM

    rep     #$30
a16()
    // Y = end of decompressed buffer
    tya
    clc
    adc.w   #-decompressionBuffer
    tay

    sep     #$20
a8()
    ldx.w   #decompressionBuffer
    lda.b   #decompressionBuffer >> 16

    jsr     Dma.ForceBlank.TransferToVram

    plb
    // DB = 0x7e

    rep     #$30
a16()
i16()
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

