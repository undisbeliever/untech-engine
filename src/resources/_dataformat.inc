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


namespace Format {

    // AnimatedTileset resource data format.
    //   <header> <lz4 static tiles + animated tiles>
    //
    // NOTE: The size of the animated tiles block must be <= MAX_ANIMATED_TILES_SIZE
    // NOTE: The number of tiles being animated must be <= MAX_TILES_PER_FRAME
    //
    namespace AnimatedTilesetHeader {
        constant MAX_TILES_PER_FRAME = 32
        constant MAX_ANIMATED_TILES_SIZE = 8192

        struct()
            // Number of animation frames
            // (uint8)
            field(nAnimations, 1)

            // Number of 4bpp tiles per animation
            // Optional field, only exists if number of animation frames > 0
            // This value MUST BE <= MAX_TILES_PER_FRAME
            // (uint8)
            field(nTiles, 1)

            // Delay between palette animation frames
            // Optional field, only exists if number of animation frames > 0
            // (uint16 - ticks)
            field(animationDelay, 2)
        endstruct()
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
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

