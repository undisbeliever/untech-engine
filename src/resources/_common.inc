// resources/_common.inc
// =====================
//
// Resource Management routines
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


RegisterBreakId(INVALID_SCENE_ID)
RegisterBreakId(INVALID_SCENE_SETTINGS_ID)
RegisterBreakId(DECOMPRESSION_BUFFER_FULL)

namespace Resources {

// Initializes the Resources module
//
// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
macro _Init() {
constant _tmp = utDpTmp0

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
    assert(decompressionBuffer == 0x7f0000)
    stz.w   decompressionBuffer

    stz.w   VramTransferBuffer.currentIndex


    AnimatedTileset._Init()


    // Disable palette data
    stz.w   Palette.animationFrameSize


    // Disable scene layout
    SceneLayout._Init()
}



// Decompress a scene to the decompression buffer
//
// This routine will not change the PPU registers, VRAM or CGRAM.
//
// Please call `TransferToPpu` after calling this routine to upload the
// decompressed graphics/settings to the PPU.
//
// NOTE: This routine will reset the following subsystems:
//        * Resources
//        * Text Console
//        * MetaSprite buffer
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: A = scene Id
a16()
i16()
code()
function LoadScene {
constant _tmp           = utDpTmp19
constant _sceneIndex    = utDpTmp18
constant _layoutTypes   = utDpTmp17
constant _layerIndex    = utDpTmp16
constant _tmBit         = utDpTmp15

    pha

    _Init()
    MetaSprite.Render._Init()

    pla


    sep     #$20
a8()
    cmp.b   #SceneData.count
    bcc     +
        break(INVALID_SCENE_ID)
    +
    sta.w   Scene.sceneId

    rep     #$30
a16()
    and.w   #0xff

    assert(Format.Scene.size == (1 << 3) - 1)
    sta.b   _tmp
    lda.b   _tmp
    asl
    asl
    asl
    sec
    sbc.b   _tmp
    sta.b   _sceneIndex
    tax

    // X = Scene data index


    // Process SceneSettings

    lda.l   Format.Scene.settings,x

    sep     #$30
a8()
a8()
    cmp.b   #SceneSettingsData.count
    bcc     +
        break(INVALID_SCENE_SETTINGS_ID)
    +
    tay
    // A/Y = scene settings id

    // MUST NOT MODIFY Y
    // Scene.functionTableIndex = sceneSettingsId * Format.SceneSettingsFunctionTable.size
    assert(Format.SceneSettingsFunctionTable.size == ((1 << 1) + 1) << 1)
    sta.w   Scene.functionTableIndex
    asl
    clc
    adc.w   Scene.functionTableIndex
    asl
    sta.w   Scene.functionTableIndex

    // x = sceneSettingsId * Format.SceneSettings.size
    assert(Format.SceneSettings.size == (1 << 2) - 1)
    tya
    sta.b   _tmp
    asl
    asl
    sec
    sbc.b   _tmp
    tax

    lda.l   Format.SceneSettings.bgMode,x
    sta.w   Scene.bgMode

    // reset Scene.tm (used in layer loop below)
    lda.b   #TM.obj
    sta.w   Scene.tm

    rep     #$30
a16()
i16()
    lda.l   Format.SceneSettings.layerTypes,x
    sta.b   _layoutTypes




    // Set Vram Layout
    ldx.b   _sceneIndex
    lda.l   Format.Scene.vramLayout,x
    SceneLayout._SetSceneLayout()


    // Load Palette
    // ::TODO add scene palette overrides::
    ldx.b   _sceneIndex
    lda.l   Format.Scene.palette,x
    jsr     Palette.LoadPalette


    // Load Layers
    stz.b   _layerIndex

    lda.w   #1
    sta.b   _tmBit

    lda.b   _layoutTypes
    beq     SkipLoop
    Loop:
        // A = _layoutTypes
        and.w   #Format.SceneSettings.LAYER_TYPES_BG_MASK
        tay
        beq     +
            // faster to use tsb in 16 bit mode then switch A size
            lda.b   _tmBit
            tsb.w   Scene.tm
        +

        ldx.b   _sceneIndex
        lda.l   Format.Scene.layers,x

        tyx
        ldy.b   _layerIndex
        jsr     (LoadLayerFunctionTable,x)


        inc.b   _sceneIndex

        assert(BG_INDEX_INCREMENT == 2)
        inc.b   _layerIndex
        inc.b   _layerIndex

        asl.b   _tmBit

        assert(Format.SceneSettings.LAYER_TYPES_BG_SHIFT == 4)
        lda.b   _layoutTypes
        lsr
        lsr
        lsr
        lsr
        sta.b   _layoutTypes
        bne     Loop

    SkipLoop:

    rts


    // Load a layer into the decompression buffer
    //
    // This routine MUST NOT use utDpTmp19 - utDpTmp15 variables
    //
    // INPUT: A = layer Id
    //        Y = bgIndex
    namespace LoadLayer {
        a16()
        i16()
        code()
        function None {
            rts
        }

        constant BackgroundImage = Resources.LoadBackgroundImage
        constant MetaTileTileset = MetaTiles.LoadTileset
        constant TextConsole     = Text.Console._EnableTextBuffer
    }

    buildFunctionTable(Format.LayerTypes, LoadLayer, LoadLayerFunctionTable)
    // add padding
    evaluate n = (Format.SceneSettings.LAYER_TYPES_BG_MASK + 2 - Format.LayerTypes.size) / 2
    while {n} > 0 {
        dw  LoadLayer.None
        evaluate n = {n} - 1
    }
    assert(Format.SceneSettings.LAYER_TYPES_BG_MASK & 1 == 0)
    assert(pc() - LoadLayerFunctionTable - 2 == Format.SceneSettings.LAYER_TYPES_BG_MASK)
    assertPowerOfTwo(pc() - LoadLayerFunctionTable)
}



// Process one display frame of the resource/scene animations.
//
// MUST be called once per frame.
//
// MUST not be called between a `LoadScene` call and a `TransferToPpu` call.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
a16()
i16()
code()
function ProcessAnimations {
    AnimatedTileset._ProcessAnimation()
    Palette._ProcessAnimation()

    sep     #$10
i8()
    assert(SceneSettingsFunctionTable.tableSize < 256)  // Confirm 8 bit indexing is allowed on function table
    ldx.w   Scene.functionTableIndex
    jsr     (Resources.Format.SceneSettingsFunctionTable.Process,x)

    rep     #$30
i16()

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
// NOTE: This routine will reset the PPU registers.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Force Blank
a16()
i16()
code()
function TransferToPpu {
constant _tmp = utDpTmp0

    pea     (0x7e << 8) | REGISTER_DB
    plb
// DB == REGISTER_DB


    // Reset Registers
    jsr     ResetRegisters

    sep     #$20
a8()
    SceneLayout._SetupPpuRegisters_RegisterDB()


    lda.l   Scene.bgMode
    sta.w   BGMODE

    lda.l   Scene.tm
    sta.w   TM


    Text.Console._TransferToPpu()


    rep     #$30
a16()

    // Transfer tile/map data to VRAM
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


    lda.w   #2100
    tcd

    sep     #$10
i8()
    assert(SceneSettingsFunctionTable.tableSize < 256)  // Confirm 8 bit indexing is allowed on function table
    ldx.w   Scene.functionTableIndex
    jsr     (Resources.Format.SceneSettingsFunctionTable.SetupPpu_dp2100,x)

    rep     #$30
i16()

    lda.w   #0
    tcd

    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

