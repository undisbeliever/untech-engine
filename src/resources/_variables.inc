// resources/_variables.inc
// ========================
//
// Variables used by the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

constant decompressionBuffer.size = 32 * 1024
allocate(decompressionBuffer, wram7f, decompressionBuffer.size)


// Number of animation ticks per display frame.
//
// This variable is set to 5 on NTSC and 6 on PAL
// This variable is shared by MetaSprite.Animation.timerSpeed
// (uint16 - ticks)
allocate(animationSpeed, wram7e, 2)


namespace AnimatedTileset {
    // buffer to hold the decompressed animated tiles
    // (wram7f)
    constant animationBuffer.size = Format.AnimatedTilesetHeader.MAX_ANIMATED_TILES_BLOCK_SIZE
    allocate(animationBuffer, wram7f, animationBuffer.size)
    assert(decompressionBuffer + decompressionBuffer.size == animationBuffer)

    // Vram word address to store the upload the buffer to
    // (vram word address)
    allocate(animationVramWaddr, shadow, 2)

    // Address of the current animation frame in the paletteBuffer
    // If 0 then animation is disabled
    // (word addr - shadow)
    allocate(animationBufferPos, shadow, 2)

    // The size of the palette animation frame in bytes
    // NOTE: After setting this value you MUST INVOKE
    //       `Dma.Resources.CalcualateDmaTime(AnimatedTileset)`
    // (uint16 - shadow)
    allocate(animationFrameSize, shadow, 2)

    // The address following the end of the data in the palette buffer
    // (word addr - shadow)
    allocate(animationBufferEnd, wram7e, 2)

    // The amount of DMA time requited to upload the animation frame to VRAM
    // (uint16)
    allocate(dmaTime, wram7e, 2)


    // Animation frame delay
    // Copy of `Format.PaletteHeader.animationDelay`
    // If zero then animation is disabled
    // (uint16 - animationTicks)
    allocate(animationDelay, wram7e, 2)

    // Animation frame countdown
    // When < 0 then a queue the next animation
    // (uint16 - animationTicks)
    allocate(animationCountdown, wram7e, 2)
}

namespace Palette {
    // memory buffer to store the decompressed palettes in
    // (wram7f)
    constant paletteBuffer.size = 32 * 128 * 2
    allocate(paletteBuffer, wram7f, paletteBuffer.size)


    // Address of the current animation frame in the paletteBuffer
    // (word addr - shadow)
    allocate(animationPos, shadow, 2)

    // The size of the palette animation frame in bytes
    // NOTE: After setting this value you MUST INVOKE
    //       `Dma.Resources.CalcualateDmaTime(Palette)`
    // (uint16 - shadow)
    allocate(animationFrameSize, shadow, 2)

    // The address following the end of the data in the palette buffer
    // (word addr - shadow)
    allocate(paletteBufferEnd, wram7e, 2)

    // The amount of DMA time required to upload the animation frame to CGRAM
    // (uint16)
    allocate(dmaTime, wram7e, 2)


    // Animation frame delay
    // Copy of `Format.PaletteHeader.animationDelay`
    // If zero then animation is disabled
    // (uint16 - animationTicks)
    allocate(animationDelay, wram7e, 2)

    // Animation frame countdown
    // When < 0 then a queue the next animation
    // (uint16 - animationTicks)
    allocate(animationCountdown, wram7e, 2)
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

