// resources/_common.inc
// =====================
//
// Resource Management routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

RegisterBreakId(DECOMPRESSION_BUFFER_FULL)

namespace Resources {

// Initializes the Resources module
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function Init {
    Palette.DisableAnimation()

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


    // Reset decompression Buffer
    lda.w   #decompressionBuffer
    sta.w   decompressionBufferPos

    stz.w   VramTransferBuffer.currentIndex


    AnimatedTileset._Init()


    // Disable palette data
    stz.w   Palette.animationFrameSize

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
    Palette._ProcessAnimation()

    rts
}



// Decompress a lz4 block into the decompressionBuffer
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// MODIFIES: decompressionBuffer
// INPUT: LZ4.ptr = address of the lz4 block
// OUTPUT: X = address at the start of the decompressed data
//         Y = address of the end of the decompressed data
//         A = size of the decompressed data
a16()
i16()
code()
function DecompressData_FromPtr {
    lda.w   #decompressionBuffer - decompressionBuffer.size
    sec
    sbc.w   decompressionBufferPos
    bcc     DecompressionBufferFull
    tay
    // Y = remaining size of buffer

    // Check we can fit the decompressed data in the decompressionBuffer
    lda.w   decompressionBufferPos
    clc
    adc     [LZ4.ptr]                   // LZ4 header: decompressed size
    bcs     DecompressionBufferFull
    cmp.w   #decompressionBuffer.size
    bcc     +
    DecompressionBufferFull:
        // Decided to do this here so I get a cleaner error message
        break(DECOMPRESSION_BUFFER_FULL)
    +

    sep     #$20
a8()

    ldx.w   decompressionBufferPos
    lda.b   #decompressionBuffer >> 16


    jsr     LZ4.Decompress


    rep     #$20
a16()
    ldx.w   decompressionBufferPos
    // X = start of the decompressed data

    // Y = end of the decompressed output buffer
    sty.w   decompressionBufferPos

    txa
    eor.w   #0xffff
    sec
    adc.w   decompressionBufferPos
    // A = size of decompressed buffer

    rts
}



// Transfer decompressed data to the PPU
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Force Blank
a16()
i16()
code()
function TransferToPpu {
    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB == REGISTER_DB

    VramLoop:
        lda.l   VramTransferBuffer.currentIndex
        beq     EndVramLoop
        dec
        dec
        sta.l   VramTransferBuffer.currentIndex
        tax


        lda.l   VramTransferBuffer.destinationVramWaddr,x
        sta.w   VMADD

        lda.l   VramTransferBuffer.size,x
        tay

        lda.l   VramTransferBuffer.sourceWram7fAddr,x
        tax

        sep     #$20
    a8()
        lda.b   #decompressionBuffer >> 16

        jsr     Dma.ForceBlank.TransferToVram

        rep     #$20
    a16()

        bra     VramLoop
EndVramLoop:


    // Transfer palette data to CGRAM
    ldy.w   Resources.Palette.animationFrameSize
    beq     NoCgramTransfer
        sep     #$20
    a8()

        stz.w   CGADD

        ldx.w   Resources.Palette.animationPos
        lda.b   #Resources.Palette.paletteBuffer >> 16

        jsr     Dma.ForceBlank.TransferToCgram

        rep     #$30
    a16()
NoCgramTransfer:

    plb
    // DB = 0x7e

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

