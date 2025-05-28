// resources/_dataformat.inc
// =========================
//
// Data format of the resources subsystem.
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


// Fixed word address of OBJ tile data in VRAM
// At the very end of the VRAM
constant VRAM_OBJ_WADDR = 0x6000


namespace Resources {


// number of animation ticks per second
constant ANIMATION_TICKS_PER_SECOND = 300


// Scene settings data
// (SceneSettings table, long addressing)
constant SceneSettingsData = Project.SceneSettingsData
constant SceneSettingsData.count = Project.SceneSettingsData.count

// Scene settings function table
// (function table, SceneSettingsFunctionTable layout)
constant SceneSettingsFunctionTable_SetupPpu_dp2100 = Project.SceneSettingsFunctionTable_SetupPpu_dp2100
constant SceneSettingsFunctionTable_Process = Project.SceneSettingsFunctionTable_Process
constant SceneSettingsFunctionTable_VBlank_dp2100 = Project.SceneSettingsFunctionTable_VBlank_dp2100

// Scene layout data
// (SceneLayout table, long addressing)
constant SceneLayoutData = Project.SceneLayoutData
constant SceneLayoutData.count = Project.SceneLayoutData.count

// Scene data
// (Scene table, long addressing)
constant SceneData = Project.SceneData
constant SceneData.count = Project.SceneData.count

// List of Background Images
// (Long Addr Table)
constant BackgroundImageList = Project.BackgroundImageList
constant BackgroundImageList.count = Project.BackgroundImageList.count

// List of palettes
// (Long Addr Table)
constant PaletteList = Project.PaletteList
constant PaletteList.count = Project.PaletteList.count


namespace Format {
    if Resources.SCENE_FORMAT_VERSION != 3 {
        error "Invalid untech-compiler Scene Format"
    }


    // SceneSettings resource data format
    namespace SceneSettings {
        struct(SceneSettingsData)
            // BG mode
            //
            // Save format as the BGMODE register
            //
            // (byte, BGMODE, lowram)
            field(bgMode, 1)

            // Layer types for each BG layer
            //
            //  ddd0 ccc0 bbb0 aaa0
            //
            //      aaa = BG1 layer type
            //      bbb = BG2 layer type
            //      ccc = BG3 layer type
            //      ddd = BG4 layer type
            //
            // See `LayerTypes` for layer values.
            //
            // (4 nibbles, `LayerTypes` enum entry)
            field(layerTypes, 2)

            constant LAYER_TYPES_BG_MASK = 0b1110
            constant LAYER_TYPES_BG_SHIFT = 4
        endstruct()
    }

    // Scene Layer Types
    namespace LayerTypes {
        // Using struct as it allows me to automatically build a function table
        basestruct()
            // The layer is unused
            field(None, 2)

            // The layer is a background image
            field(BackgroundImage, 2)

            // ::TODO add AnimatedImage::

            // The layer is an MetaTile Tileset
            // (NOTE: a maximum of one MetaTileTileset per scene is allowed, 2)
            field(MetaTileTileset, 2)

            // Text.Console layer
            // (NOTE: a maximum of one Text Console per scene is allowed, 2)
            field(TextConsole, 2)

            // ::TODO add StatusBar::
        endstruct()

        assert(size <= SceneSettings.LAYER_TYPES_BG_MASK)
    }


    // SceneSettings Function Tables.
    namespace SceneSettingsFunctionTable {
        // Scene PPU setup routine function Table.
        //
        // This function is called by `TransferToPpu` and is used to
        // setup the PPU registers for the scene.
        //
        // REGS: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0x2100
        constant SetupPpu_dp2100 = Project.SceneSettingsFunctionTable_SetupPpu_dp2100

        // Scene process routine.
        //
        // Called once per frame by `Resources.ProcessAnimations`
        //
        // REGS: 16 bit A, 8 bit Index, DB = 0x7e, DP = 0
        constant Process = Project.SceneSettingsFunctionTable_Process

        // Scene VBlank routine.
        //
        // Called once per frame during VBlank.
        //
        // REGS: 16 bit A, 8 bit Index, DB = 0x80, DP = 0
        constant VBlank_dp2100 = Project.SceneSettingsFunctionTable_VBlank_dp2100


        constant tableSize = Project.SceneSettingsFunctionTable_Process.size
        constant elementSize = 2

        assert(Project.SceneSettingsFunctionTable_SetupPpu_dp2100.size == SceneSettingsData.count * 2)
        assert(Project.SceneSettingsFunctionTable_Process.size         == SceneSettingsData.count * 2)
        assert(Project.SceneSettingsFunctionTable_VBlank_dp2100.size   == SceneSettingsData.count * 2)

        assert(Project.SceneSettingsFunctionTable_Process.size < 256)  // allow 8 bit indexing
    }



    // SceneLayout resource data format
    namespace SceneLayout {
        struct(SceneLayoutData)
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



    // Scene resource data format
    namespace Scene {
        struct(SceneData)
            // Scene Settings id (`SceneSettings` struct)
            //
            // (byte)
            field(settings, 1)

            // Vram Layout id (`SceneLayout` struct)
            // (byte)
            field(vramLayout, 1)

            // Palette Id (`PaletteHeader` struct)
            // (byte)
            field(palette, 1)

            // Layer id for each Background layer.
            //
            // The type of each entry is determined by the `SceneSettings` struct.
            // See `SceneSettings.layerTypes` for more details.
            //
            // (4x bytes)
            field(layers, 4)
        endstruct()
    }



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

