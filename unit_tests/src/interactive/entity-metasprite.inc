// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace Entities {

namespace ShowAllFrames {
    buildFunctionTable(BaseEntityFunctionTable, ShowAllFrames)

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(frameId, 2)
            field(paletteId, 2)
            field(frameCount, 2)

            field(frameDelay, 2)
        endstruct()
    }

    // IN: A = frameSet to render (entity parameter)
    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        ldy.b   EntityStruct.MetaSprite.palette
        jsr     MetaSpriteTest._InitEntity

        lda.w   #10
        sta.b   EntityStruct.frameDelay

        lda.w   #-1
        sta.b   EntityStruct.frameCount
        sta.b   EntityStruct.frameId

        stz.b   EntityStruct.paletteId

        // always activate
        sec
        rts
    }

    a16()
    i16()
    code()
    Destructor:
    Deactivated:
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

        jmp     Entity.DrawAndGotoNextEntity
    }


    rodata(EN_RomData)
    RomData:
        dw  FunctionTable
        db  EntityLists.Players     // initialListId
        db  0                       // defaultPalette
        dw  MSFS.FST_OneTile        // frameSetId

    rodata(EN_EntityDataList)
    constant EntityDataId = (pc() - Entity.Data.EntityDataList) / 2
        dw  RomData
}
}

namespace InteractiveTests {
namespace MetaSpriteInteractiveTest {

rodata(rom0)
namespace EntityTable {
    struct(pc())
        field(frameSet, 2)
        field(delay, 2)
        field(xPos, 2)
        field(yPos, 2)
    endstruct()

    macro data(frameSet, frameDelay, xPos, yPos) {
        dw  MSFS.{frameSet}
        dw  {frameDelay}
        dw  {xPos} + Camera.ORIGIN
        dw  {yPos} + Camera.ORIGIN
    }

Data:
    data(Man,               10,     50,         200)
    data(SpikedWalker,      10,     128,        200)
    data(Woman,             10,     256 - 50,   200)

    data(FST_OneTileFixed,  40,     40,         150)
    data(FST_TwoTilesFixed, 40,     256 - 40,   150)

    data(FST_OneTile,       150,    40,         100)
    data(FST_TwoTiles,      150,    256 - 40,   100)

    data(FST_TwoRows,       200,    256 / 2,    45)

    constant N_ENTITIES = (pc() - Data) / EntityTable.size
}


// Spawns the entities used by the test
// OUTPUT: c set if successful
a16()
i16()
code()
function SpawnEntities {
constant tmp_index = Test.tmp

    assert(EntityTable.N_ENTITIES > 0)
    ldx.w   #0

    Loop:
        stx.w   tmp_index

        lda.l   EntityTable.xPos,x
        sta.w   Entity.Spawn.xPos

        lda.l   EntityTable.yPos,x
        sta.w   Entity.Spawn.yPos

        lda.l   EntityTable.frameSet,x
        tay

        lda.w   #Entities.ShowAllFrames.EntityDataId

        jsr     Entity.Spawn
        bcc     Fail

        // Update frame delay
        // Y = newly spawned entity
        ldx.w   tmp_index
        lda.l   EntityTable.delay,x
        sta.w   Entities.ShowAllFrames.EntityStruct.frameDelay,y

        txa
        clc
        adc.w   #EntityTable.size
        tax

        cpx.w   #EntityTable.size * EntityTable.N_ENTITIES
        bcc     Loop

    sec
    rts


Fail:
    clc
    rts
}



// The Test
a16()
i16()
code()
Test.add("Entity/MetaSprite Test")
function Test {
    jsr     EntityLoop.Init

    jsr     EnableDisplay_Full


    jsr     SpawnEntities
    bcc     Fail

    jmp     EntityLoop.ProcessUntilButtonPress


Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

