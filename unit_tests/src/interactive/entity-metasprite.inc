// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace MetaSpriteInteractiveTest {

namespace ShowAllFramesEntity {
    buildFunctionTable(BaseEntityFunctionTable, ShowAllFramesEntity)
    Entity.useDefaultRenderer()

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(frameId, 2)
            field(paletteId, 2)
            field(frameCount, 2)

            field(frameDelay, 2)
        endstruct()
    }

    // IN: A = parameter (frameDelay)
    a16()
    i16()
    code()
    function Init {
        sta.b   EntityStruct.frameDelay
        sta.b   EntityStruct.frameCount

        lda.w   #-1
        sta.b   EntityStruct.frameCount
        sta.b   EntityStruct.frameId

        stz.b   EntityStruct.paletteId

        rts
    }

    a16()
    i16()
    code()
    Destructor:
    ProcessCollision:
    function Null {
        rts
    }


    a16()
    i16()
    code()
    function Process {
        dec.b   EntityStruct.frameCount
        bpl     EndIf

            sep     #$20
        a8()
            ldx.b   EntityStruct.MetaSprite.frameSet
            ldy.b   EntityStruct.paletteId

            lda.b   EntityStruct.frameId
            inc
            cmp.l   MetaSprite.Format.FrameSet.nFrames,x
            bcc     ++
                // increment palette when resetting to frame 0
                tya
                inc
                cmp.l   MetaSprite.Format.FrameSet.nPalettes,x
                bcc     +
                    lda.b   #0
                +
                tay

                lda.b   #0
            +

            // A = frame Id
            // Y = palette Id

            rep     #$30
        a16()
            and.w   #$00ff
            pha
            phy
                jsr     MetaSprite.SetFrame
                bcc     RestoreStack

                sep     #$20
            a8()
                lda     1,s
                jsr     MetaSprite.Palette.SetPalette

                rep     #$30
            a16()
        RestoreStack:
            ply
            pla
            bcc     EndIf

                // frameCount is only reset upon a successful SetFrame
                // so that cases where we run out of DMA time are handled.
                sta.b   EntityStruct.frameId
                sty.b   EntityStruct.paletteId

                lda.b   EntityStruct.frameDelay
                sta.b   EntityStruct.frameCount
        EndIf:
    a16()

        jmp     Entity.GotoNextEntity
    }


    inline BuildEntityData(name) {
        rodata(EN_RomData)
        RomData_{name}:
            dw  FunctionTable
            db  Entity.ListId.PLAYER    // initial List
            dw  MSFS.{name}             // frameSetId
            db  0                       // defaultPalette

        rodata(EN_EntityList)
        constant {name}_entityId = (pc() - Entity.Data.EntityList) / 2
            dw  RomData_{name}
    }
    BuildEntityData(Man)
    BuildEntityData(SpikedWalker)
    BuildEntityData(Woman)
    BuildEntityData(FST_OneTileFixed)
    BuildEntityData(FST_TwoTilesFixed)
    BuildEntityData(FST_OneTile)
    BuildEntityData(FST_TwoTiles)
    BuildEntityData(FST_TwoRows)
}

rodata(rom0)
namespace EntityTable {
    struct(pc())
        field(entityId, 2)
        field(parameter, 2)
        field(xPos, 2)
        field(yPos, 2)
    endstruct()

    macro data(eName, frameDelay, xPos, yPos) {
        dw  ShowAllFramesEntity.{eName}_entityId
        dw  {frameDelay}
        dw  {xPos} + Camera.ORIGIN
        dw  {yPos} + Camera.ORIGIN
    }

Data:
    data(Man, 10, 50, 200)
    data(SpikedWalker, 10, 128, 200)
    data(Woman, 10, 256 - 50, 200)

    data(FST_OneTileFixed, 40, 40, 150)
    data(FST_TwoTilesFixed, 40, 256 - 40, 150)

    data(FST_OneTile, 150, 40, 100)
    data(FST_TwoTiles, 150, 256 - 40, 100)

    data(FST_TwoRows, 200, 256 / 2, 45)

    constant N_ENTITIES = (pc() - Data) / EntityTable.size
}


// The Test
a16()
i16()
code()
Test.add("Entity/MetaSprite Test")
function Test {
constant tmp_index = Test.tmp

    jsr     EntityLoop.Init


    // Load the entities
    // -----------------
    assert(EntityTable.N_ENTITIES > 0)
    ldx.w   #0

    Loop:
        stx.w   tmp_index

        lda.l   EntityTable.xPos,x
        sta.w   Entity.Spawn.xPos

        lda.l   EntityTable.yPos,x
        sta.w   Entity.Spawn.yPos

        lda.l   EntityTable.parameter,x
        tay
        lda.l   EntityTable.entityId,x

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp_index
        clc
        adc.w   #EntityTable.size
        tax

        cpx.w   #EntityTable.size * EntityTable.N_ENTITIES
        bcc     Loop


    jmp     EntityLoop.ProcessUntilButtonPress

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

