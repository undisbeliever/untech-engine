// resources/_dataformat.inc
// =========================
//
// Data format of the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Resources {

// The Resources subsystem requires the following to be defined:
//
//   Resources.PaletteList
//      a list of long pointers that point to Resources.Format.Palette data
//
//   Resources.PaletteList.count
//      the number of elements in the PaletteList
//


// number of animation ticks per second
constant ANIMATION_TICKS_PER_SECOND = 300


// List of palettes
// (Long Addr Table)
constant PaletteList = Project.PaletteList
constant PaletteList.count = Project.PaletteList.count

namespace Format {

    // AnimatedTileset resource data format.
    //   <header> <lz4 static tiles + animated tiles>
    //
    // NOTE: The size of the animated tiles block must be <= MAX_ANIMATED_TILES_BLOCK_SIZE
    // NOTE: The size of the animated tiles frame must be <= MAX_ANIMATION_FRAME_SIZE
    //
    namespace AnimatedTilesetHeader {
        constant MAX_ANIMATED_TILES_BLOCK_SIZE = 8192
        constant MAX_ANIMATION_FRAME_SIZE = 32 * 32 // 32 4bpp tiles
        constant ANIMATION_FRAME_SIZE_SCALE = 16

        assert(MAX_ANIMATION_FRAME_SIZE / ANIMATION_FRAME_SIZE_SCALE < 255)

        struct()
            // Number of animation frames
            // (uint8)
            field(nAnimations, 1)

            // Size of an animation frame, scaled by ANIMATION_FRAME_SIZE_SCALE.
            // Optional field, only exists if number of animation frames > 0
            //
            // This value is scaled so it can fit inside an 8 bit integer.
            // The number of bytes transferred per frame is:
            //     `animationFrameSizeScaled` * ANIMATION_FRAME_SIZE_SCALE
            //
            // This value MUST BE <= MAX_ANIMATION_FRAME_SIZE / ANIMATION_FRAME_SIZE_SCALE
            // (uint8)
            field(animationFrameSizeScaled, 1)

            // Delay between palette animation frames
            // Optional field, only exists if number of animation frames > 0
            // (uint16 - ticks)
            field(animationDelay, 2)
        endstruct()
    }

    if Resources.ANIMATED_TILESET_FORMAT_VERSION != 2 {
        error "Invalid untech-compiler Animated Tileset Format"
    }


    // Palette resource data format.
    //   <header> <lz4 compressed snes colors>
    //
    // Palettes can be animated, with a new palette uploaded to CGRAM every
    // `animationDelay/ANIMATION_TICKS_PER_SECOND` seconds.
    // The number of colors in each animation frame must be constant and is
    // calculated from `<lz4 decompressed size> / <nAnimations> / 2`.
    namespace PaletteHeader {
        struct()
            // Number of animations in this palette
            // (uint8)
            field(nAnimations, 1)

            // Delay between palette animation frames
            // Optional field, only exists if `nAnimations > 0`
            // (uint16 - ticks)
            field(animationDelay, 2)
        endstruct()
    }

    if Resources.PALETTE_FORMAT_VERSION != 2 {
        error "Invalid untech-compiler Palette Format"
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

