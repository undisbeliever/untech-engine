// resources/_variables.inc
// ========================
//
// Variables used by the resources subsystem.
//
// SPDX-FileCopyrightText: © 2017 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2017 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


// Triggered when the VramTransferBuffer overflows
RegisterBreakId(TOO_MANY_RESOURCES)

namespace Resources {

// Decompression buffer
//
// Holds the decompressed tile data used by the resources subsystem.
//
// Must call `Resources.Init` to reset the buffer position
// (wram 7F)
constant decompressionBuffer.size = 48 * 1024
allocate(decompressionBuffer, wram7f, decompressionBuffer.size)

// Current location within the decompression buffer
// (word pointer)
allocate(decompressionBufferPos, wram7e, 2)


// Number of animation ticks per display frame.
//
// This variable is set to 5 on an NTSC console or 6 on a PAL console
// This variable is shared by MetaSprite.Animation.timerSpeed
// (uint16 - ticks)
allocate(animationSpeed, wram7e, 2)


namespace Scene {
    // SceneSettings Function Table Index
    // See `Resources.Format.SceneSettingsFunctionTable_*`
    // (byte index, lowram)
    allocate(functionTableIndex, lowram, 1)


    // Current scene ID
    // (byte)
    allocate(sceneId, wram7e, 1)

    // BGMODE register buffer
    // (byte)
    allocate(bgMode, wram7e, 1)

    // TM register buffer
    //
    // Dynamically created depending on SceneSettings selected
    //
    // (byte)
    allocate(tm, wram7e, 1)
}


namespace SceneLayout {
    // Byte Index of the current SceneLayout data
    //
    // Equal to `sceneId * Format.SceneLayout.size`
    //
    // (word index, lowram)
    allocate(dataIndex, lowram, 2)
}
constant BG_INDEX_MASK = Format.SceneLayout.BG_INDEX_MASK
constant BG_INDEX_INCREMENT = Format.SceneLayout.BG_INDEX_INCREMENT

namespace AnimatedTileset {
    // BG layer used by the animated tileset
    //
    // This variable MUST BE masked by BG_INDEX_MASK before storing it.
    //
    // (byte index inc2, lowram)
    allocate(bgIndex, lowram, 1)

    // VRAM word address of the tilemap of the BG used by the animated BG
    // (vram word address)
    allocate(tilemapVramWaddr, wram7e, 2)

    // The BG scroll register values used by the animated tileset
    // (2 * uint16, lowram)
    allocate(displayHoffset, lowram, 2)
    allocate(displayVoffset, lowram, 2)


    // Vram word address to store the upload the buffer to
    // (vram word address)
    allocate(animationVramWaddr, lowram, 2)

    // The size of the animation frame in bytes
    // NOTE: After setting this value you MUST INVOKE
    //       `Dma.Resources.CalcualateDmaTime(AnimatedTileset)`
    // (uint16 - lowram)
    allocate(animationFrameSize, lowram, 2)


    // Address of the current animation frame
    // If 0 then animation is disabled
    // (word addr - lowram)
    allocate(animationBufferPos, lowram, 2)

    // The starting address of the animation buffer
    // (word addr - lowram)
    allocate(animationBufferBegin, wram7e, 2)

    // The address following the end of the data in the animation buffer
    // (word addr - lowram)
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
    // (word addr - lowram)
    allocate(animationPos, lowram, 2)

    // The size of the palette animation frame in bytes
    // NOTE: After setting this value you MUST INVOKE
    //       `Dma.Resources.CalcualateDmaTime(Palette)`
    // (uint16 - lowram)
    allocate(animationFrameSize, lowram, 2)

    // The address following the end of the data in the palette buffer
    // (word addr - lowram)
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


// Buffer of DMA transfers from WRAM 7F to VRAM
namespace VramTransferBuffer {
    constant N_ELEMENTS = 10

    // current index within buffer
    // (word)
    allocate(currentIndex, wram7e, 2)

    constant LAST_INDEX = N_ELEMENTS * 2


    // Location of data in bank wram7f
    // (word ptr array)
    allocate(sourceWram7fAddr, wram7e, 2 * N_ELEMENTS)

    // size of transfer
    // (uint16 array)
    allocate(size, wram7e, 2 * N_ELEMENTS)

    // vram word address to upload data to
    // (uint16 array - word address)
    allocate(destinationVramWaddr, wram7e, 2 * N_ELEMENTS)
}

}


namespace Scenes {
    // Scene Setting Function variables
    // (7x word, lowram)
    allocate(sceneVar0, lowram, 2)
    allocate(sceneVar1, lowram, 2)
    allocate(sceneVar2, lowram, 2)
    allocate(sceneVar3, lowram, 2)
    allocate(sceneVar4, lowram, 2)
    allocate(sceneVar5, lowram, 2)
    allocate(sceneVar6, lowram, 2)
    allocate(sceneVar7, lowram, 2)
}

// vim: ft=bass-65816 ts=4 sw=4 et:

