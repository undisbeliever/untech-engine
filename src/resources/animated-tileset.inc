// resources/animated-tileset.inc
// ==============================
//
// Animated Tileset resource management.
//
// Only one animated tileset can be active at any given time.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ANIMATED_TILESET)

namespace Resources {
namespace AnimatedTileset {


// Load Animated tileset from a given ptr to a given word address.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0, interrupts disabled, force blank
// DMA: Uses DMA channel 0
// MODIFIES: decompressionBuffer
//
// INPUT: LZ4.ptr = address of the lz4 block
//              X = vram word address to load tiles to
a16()
i16()
code()
function LoadAnimatedTileset_FromPtr {
constant _ptr = LZ4.ptr
constant _staticTilesSize = dpTmp0
constant _staticTilesPtr = dpTmp1

assert(Format.AnimatedTilesetHeader.size == 4)
assert(decompressionBuffer + decompressionBuffer.size == animationBuffer)

    ldy.b   _ptr

    lda     [_ptr]                  // nAnimations/nTiles
    iny
    bit.w   #0xff
    bne     HasAnimation
        // No Animation, skip header, disable animation and compress to VRAM

        sty.b   _ptr
        stz.w   animationBufferPos
        // X = vram word address
        jmp     DecompressToVram_FromPtr

HasAnimation:
    iny
    sty.b   _ptr

    sta.l   WRMPYA                  // also set WRMPYB

    and.w   #0xff00
    lsr
    lsr
    lsr
    // A = nTiles * 32

    cmp.w   #Format.AnimatedTilesetHeader.MAX_TILES_PER_FRAME * 32 + 1
    bcc     +
        // too many tiles per frame
    Fail:
        break(INVALID_ANIMATED_TILESET)
    +
    sta.w   animationFrameSize

    lda.w   #animationBuffer
    sta.w   animationBufferPos

    lda     [_ptr]                  // animationDelay
    sta.w   animationDelay
    sta.w   animationCountdown

    iny
    iny
    sty.b   _ptr


    // Calculate location to decompress the buffer to.
    // Align _staticTilesPtr so that the animated tiles start at animationBuffer

    //  RDMPY = nTiles * nFrames  (from above)
    //  if (RDMPY > animationBuffer.size / 32):
    //      goto Fail
    //
    //  _staticTilesSize = decompressedDataSize - RDMPY * 32
    //  _staticTilesPtr = animationBuffer - _staticTilesPtr
    //
    //  if _staticTilesPtr < decompressionBuffer:
    //      goto Fail

    lda.l   RDMPY
    cmp.w   #animationBuffer.size / 32
    bcs     Fail
    asl
    asl
    asl
    asl
    asl

    // A = size of animation Block
    eor.w   #0xffff
    sec
    adc     [_ptr]                  // lz4 decompressed size
    bcc     Fail                    // error: underflow
    sta.b   _staticTilesSize

    eor.w   #0xffff
    sec
    adc.w   #animationBuffer
    bcc     Fail                    // error: underflow
    assert(decompressionBuffer & 0xffff == 0)
    sta.b   _staticTilesPtr


    // save VRAM word address
    txa
    sta.l   VMADD

    // animationVramWaddr = VMADD + _staticTilesSize / 2
    asl
    clc
    adc.b   _staticTilesSize
    lsr
    sta.w   animationVramWaddr


    // Decompress data block to _staticTilesPtr

    ldx.b   _staticTilesPtr
    lda.w   #decompressionBuffer >> 16
    ldy.w   #decompressionBuffer.size + animationBuffer.size

    jsr     LZ4.Decompress

    sty.w   animationBufferEnd



    // Transfer static tiles and first frame of animated tiles to VRAM

    pea     (0x7e << 8) | REGISTER_DB
    plb
    // DB == REGISTER_DB

    // transfer size = _staticTilesSize + animatedFrameSize
    lda.b   _staticTilesSize
    clc
    adc.w   animationFrameSize
    tay

    sep     #$20
a8()
    ldx.b   _staticTilesPtr
    lda.b   #decompressionBuffer >> 16

    jsr     Dma.ForceBlank.TransferToVram

    plb
    // DB = 0x7e

    rep     #$30
a16()
    rts
}


// Disables the palette animation
// REQUIRES: 16 bit A, DB access shadow
macro DisableAnimation() {
    assert16a()

    stz.w   Resources.AnimatedTileset.animationBufferPos
}



// Processes one display frame of the tileset animation (if enabled).
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
macro _ProcessAnimation() {
    assert16a()
    assert16i()

    ldy.w   AnimatedTileset.animationBufferPos
    beq     Return
        lda.w   AnimatedTileset.animationCountdown
        sec
        sbc.w   animationSpeed
        bcs     AnimationUnchanged

            Dma.Resources.TestDmaTimeAndSetUploadFlag(AnimatedTileset, NotEnoughDmaTime)

            // Y = animationBufferPos
            tya
            clc
            adc.w   AnimatedTileset.animationFrameSize
            cmp.w   AnimatedTileset.animationBufferEnd
            bcc     +
                lda.w   #AnimatedTileset.animationBuffer
            +
            sta.w   AnimatedTileset.animationBufferPos

            lda.w   AnimatedTileset.animationDelay

    AnimationUnchanged:
        sta.w   AnimatedTileset.animationCountdown

    NotEnoughDmaTime:
Return:
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

