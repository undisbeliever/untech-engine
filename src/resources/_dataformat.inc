// resources/_dataformat.inc
// =========================
//
// Data format of the resources subsystem.
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Fixed word address of OBJ tile data in VRAM
// At the very end of the VRAM
constant VRAM_OBJ_WADDR = 0x6000


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

// Scene layout data
// (Long SceneLayout table)
constant SceneLayoutData = Project.SceneLayout
constant SceneLayoutData.count = Project.SceneLayout.count

// List of Background Images
// (Long Addr Table)
constant BackgroundImageList = Project.BackgroundImageList
constant BackgroundImageList.count = Project.BackgroundImageList.count

// List of palettes
// (Long Addr Table)
constant PaletteList = Project.PaletteList
constant PaletteList.count = Project.PaletteList.count


namespace Format {
    // SceneLayout resource data format
    namespace SceneLayout {
        struct(SceneLayoutData)
            // BG mode
            //
            // Save format as the BGMODE register
            //
            // (byte, BGMODE, shadow ram)
            field(bgMode, 1)


            // Background tilemap and character locations
            //
            // Two bytes per BG:
            //      Odd bytes  - Same format as BGxSC registers
            //      Even bytes - Base character word address / BG12NBA.walign
            //
            // NOTE: This field is a word based SOA is used so the `bgIndex`
            //       variable increments by 2 for each BG and can be used to
            //       index the scroll buffer.
            //
            // (4 * word)
            field(backgroundSettings, 2 * 4)

            constant bg1Sc  = backgroundSettings + 0
            constant bg2Sc  = backgroundSettings + 2
            constant bg3Sc  = backgroundSettings + 4
            constant bg4Sc  = backgroundSettings + 6

            constant bg1Nba = backgroundSettings + 1
            constant bg2Nba = backgroundSettings + 3
            constant bg3Nba = backgroundSettings + 5
            constant bg4Nba = backgroundSettings + 7

            // Mask to show valid bgIndex values
            constant BG_INDEX_MASK = 0x3 * 2
            constant BG_INDEX_INCREMENT = 2
        endstruct()
    }
    // ::TODO add Resources.SCENE_LAYOUT_FORMAT_VERSION test::



    // AnimatedTileset resource data format.
    //   <header> <lz4 static tiles + animated tiles>
    //
    // NOTE: The size of the animated tiles frame must be <= MAX_ANIMATION_FRAME_SIZE
    //
    namespace AnimatedTilesetHeader {
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



    // BackgroundImage resource data format.
    //  <header> <lz4 of concatenated tilemap + tile data>
    namespace BackgroundImageHeader {
        constant MAX_N_TILEMAPS = 16
        constant TILEMAP_SIZE = 32 * 32 *2

        struct()
            // Tilemap count
            //
            //  0000mmmm
            //
            //      mmmm = number of tilemaps - 1
            //
            // (byte)
            field(tilemapCount, 1)

            constant tilemapCount.mask  = 0b00001111
            assert(MAX_N_TILEMAPS - 1 == tilemapCount.mask)
        endstruct()
    }
    if Resources.BACKGROUND_IMAGE_FORMAT_VERSION != 1 {
        error "Invalid untech-compiler Background Image Format"
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

