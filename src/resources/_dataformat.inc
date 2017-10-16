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

