// resources/animated-tileset.inc
// ==============================
//
// Animated Tileset resource management.
//
// Only one animated tileset can be active at any given time.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_ANIMATED_TILESET)

namespace Resources {
namespace AnimatedTileset {


// Initialize the AnimatedTileset subsystem
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert16i()

    AnimatedTileset.DisableAnimation()

    stz.w   AnimatedTileset.bgIndex

    stz.w   AnimatedTileset.displayHoffset
    lda.w   #-1
    sta.w   AnimatedTileset.displayVoffset
}



// Load Animated tileset from a given ptr to a given word address.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// MODIFIES: decompressionBuffer
//
// INPUT: LZ4.ptr = address of the lz4 block
//        AnimatedTileset.bgIndex = bgIndex for the animated tileset
//
// OUTPUT: AnimatedTileset.tilemapVramWaddr = VRAM word address for the tile map used by BgLayer
a16()
i16()
code()
function LoadAnimatedTileset_FromPtr {
constant _ptr = LZ4.ptr
constant _animatedTilesSize = utDpTmp0
constant _vramUploadSize = utDpTmp1
constant _vramTileWaddr = utDpTmp2

assertZeroPage(_ptr)
assert(Format.AnimatedTilesetHeader.size == 4)


    lda.w   bgIndex
    jsr     SceneLayout.CalcTileAndMapVramWaddr
    sta.b   _vramTileWaddr
    sty.w   tilemapVramWaddr


    ldy.b   _ptr

    assert(Format.AnimatedTilesetHeader.nAnimations == 0)
    assert(Format.AnimatedTilesetHeader.animationFrameSizeScaled == 1)
    lda     [_ptr]                  // nAnimations & animationFrameSizeScaled
    iny
    bit.w   #0xff
    bne     HasAnimation
        // No Animation, skip header, disable animation and compress to VRAM

        sty.b   _ptr
        stz.w   animationBufferPos

        jsr     DecompressData_FromPtr
        bra     AddToVramTransferBuffer

HasAnimation:
    iny
    sty.b   _ptr

    sta.l   WRMPYA                  // also set WRMPYB

    assert(Format.AnimatedTilesetHeader.ANIMATION_FRAME_SIZE_SCALE == 16)
    and.w   #0xff00
    lsr
    lsr
    lsr
    lsr
    // A = animationFrameSizeScaled * ANIMATION_FRAME_SIZE_SCALE

    cmp.w   #Format.AnimatedTilesetHeader.MAX_ANIMATION_FRAME_SIZE + 1
    bcc     +
        // too many tiles per frame
    Fail:
        break(INVALID_ANIMATED_TILESET)
    +
    sta.w   animationFrameSize
    Dma.Resources.CalcualateDmaTime(AnimatedTileset)

    assert(Format.AnimatedTilesetHeader.animationDelay == 2)
    lda     [_ptr]                  // animationDelay
    sta.w   animationDelay
    sta.w   animationCountdown

    iny
    iny
    sty.b   _ptr


    // Calculate size of animated tiles
    //  RDMPY = nTiles * animationFrameSizeScaled  (from above)
    // _animatedTilesSize = RDMPY * ANIMATION_FRAME_SIZE_SCALE

    assert(Format.AnimatedTilesetHeader.ANIMATION_FRAME_SIZE_SCALE == 16)
    lda.l   RDMPY
    asl
    asl
    asl
    asl
    sta.b   _animatedTilesSize


    jsr     DecompressData_FromPtr


    // X = start of decompression buffer
    // Y = end of decompression buffer
    // A = size of decompression buffer

    // MUST KEEP X

    // _vramUploadSize = decompressedSize - _animatedTilesSize + animationFrameSize
    sec
    sbc.b   _animatedTilesSize
    // A = size of static tiles
    clc
    adc.w   animationFrameSize
    sta.b   _vramUploadSize

    // set animationVramWaddr
    sec
    sbc.w   animationFrameSize
    // A = size of static tiles
    lsr
    clc
    adc.b   _vramTileWaddr
    sta.w   animationVramWaddr

    // Set animation buffer end/begin/pos
    sty.w   animationBufferEnd
    tya
    sec
    sbc.b   _animatedTilesSize
    sta.w   animationBufferBegin
    sta.w   animationBufferPos
    bne     +
        // This should never happen.
        // Animated tileset should have at least one static tile.
    InvalidAnimationBufferPos:
        break(INVALID_ANIMATED_TILESET)
    +

    lda.b   _vramUploadSize


AddToVramTransferBuffer:
    // Add tiles to DMA buffer

    // X = starting address
    // A = bytes to transfer

    ldy.w   VramTransferBuffer.currentIndex
    cpy.w   #VramTransferBuffer.LAST_INDEX
    bcc     +
        break(TOO_MANY_RESOURCES)
    +

    sta.w   VramTransferBuffer.size,y

    txa
    sta.w   VramTransferBuffer.sourceWram7fAddr,y

    lda.b   _vramTileWaddr
    sta.w   VramTransferBuffer.destinationVramWaddr,y

    iny
    iny
    sty.w   VramTransferBuffer.currentIndex

    rts
}


// Disables the animated tileset animation
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
                lda.w   AnimatedTileset.animationBufferBegin
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

