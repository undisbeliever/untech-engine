// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
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
                SetFrame_A()
                bcc     RestoreStack

                sep     #$20
            a8()
                lda     1,s
                SetPalette_A()

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
        dw  {xPos} + MetaTiles.map.LEFT
        dw  {yPos} + MetaTiles.map.TOP
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

        lda.w   #Project.EntityIds.ShowAllFrames

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
    lda.w   #Project.RoomList.EmptyRoom
    ldx.w   #Project.PlayerIds.Blank
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    jsr     Resources.TransferToPpu
    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far

    jsr     Test.PrintCurrentTestName

    jsr     EnableDisplay_Full


    jsr     SpawnEntities
    bcc     Fail


    jmp     GameLoop.ProcessUntilButtonPress


Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

