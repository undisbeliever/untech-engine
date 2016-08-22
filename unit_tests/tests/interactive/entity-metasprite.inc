
scope InteractiveTests {
scope MetaSpriteInteractiveTest {

scope ShowAllFramesEntity {
    buildFunctionTable(BaseEntityFunctionTable, ShowAllFramesEntity)
    Entities.useDefaultRenderer()

    scope EntityStruct: {
        childstruct(BaseEntity)
            field(frameId, 2)
            field(frameCount, 2)

            field(frameDelay, 2)
        endstruct()
    }

    // IN: A = parameter (frameDelay)
    a16()
    i16()
    code()
    scope Init: {
        sta.b   EntityStruct.frameDelay
        sta.b   EntityStruct.frameCount

        lda.w   #-1
        sta.b   EntityStruct.frameCount
        sta.b   EntityStruct.frameId

        rts
    }

    a16()
    i16()
    code()
    scope Destructor: {
        rts
    }


    a16()
    i16()
    code()
    scope Process: {
        dec.b   EntityStruct.frameCount
        bpl     EndIf

            sep     #$20
        a8()
            ldx.b   EntityStruct.MetaSprite.frameSet

            lda.b   EntityStruct.frameId
            inc
            cmp.l   MetaSprite.Format.FrameSet.nFrames,x
            bcc     +
                lda.b   #0
            +

            rep     #$20
        a16()
            and.w   #$00ff
            pha
                jsr     MetaSprite.SetFrame
            pla
            bcc     EndIf

                // frameCount is only reset upon a successful SetFrame
                // so that cases where we run out of DMA time are handled.
                sta.b   EntityStruct.frameId

                lda.b   EntityStruct.frameDelay
                sta.b   EntityStruct.frameCount
        EndIf:
    a16()
        rts
    }


    macro BuildEntityData(name) {
        rodata(EN_RomData)
        scope RomData_{name}: {
            dw  FunctionTable
            db  0               // initial List (player) ::TODO name these lists::
            dw  MSFS.{name}     // frameSetId
            db  0               // defaultPalette
        }

        rodata(EN_RomDataList)
        constant {name}_entityId((pc() - Entities.Data.List) / 2)
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
scope EntityTable: {
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

    data(Man, 10, 50, 200)
    data(SpikedWalker, 10, 128, 200)
    data(Woman, 10, 256 - 50, 200)

    data(FST_OneTileFixed, 40, 40, 150)
    data(FST_TwoTilesFixed, 40, 256 - 40, 150)

    data(FST_OneTile, 150, 40, 100)
    data(FST_TwoTiles, 150, 256 - 40, 100)

    data(FST_TwoRows, 200, 256 / 2, 45)

constant N_ENTITIES((pc() - EntityTable) / EntityTable.size)
}

// Initialize the test
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
macro Init() {
    assert16a()
    assert16i()

}


// The Test
code()
a16()
i16()
Test.add("Entity/MetaSprite Test")
scope Test: {
constant tmp_index(Test.tmp)

    // Setup The screen
    // ----------------
    pea     (0x7e << 8) | REGISTER_DB
    plb
// DB = REGISTER_DB

    sep     #$20
a8()
    lda.b   #INIDISP.force
    sta.w   INIDISP

    assert(VRAM_OBJ_WADDR % OBSEL.base.walign == 0)

    lda.b   #(VRAM_OBJ_WADDR / OBSEL.base.walign) | OBSEL.size.s8_16
    sta.w   OBSEL

    lda.b   #TM.bg1 | TM.obj
    sta.w   TM

    jsr     EnableVBlank

    lda.b   #15
    sta.w   INIDISP

    rep     #$30
a16()
    plb
// DB = 0x7e


    // Setup the modules
    // -----------------
    jsr     Entities.Init


    // Wait for button release
    // -----------------------
    WaitButtonRelease:
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     WaitButtonRelease


    // Load the entities
    // -----------------
    assert(EntityTable.N_ENTITIES > 0)
    ldx.w   #0

    Loop:
        stx.w   tmp_index

        lda.l   EntityTable.xPos,x
        sta.w   Entities.Spawn.xPos

        lda.l   EntityTable.yPos,x
        sta.w   Entities.Spawn.yPos

        lda.l   EntityTable.parameter,x
        tay
        lda.l   EntityTable.entityId,x

        jsr     Entities.Spawn
        bcc     Fail

        lda.w   tmp_index
        clc
        adc.w   #EntityTable.size
        tax

        cpx.w   #EntityTable.size * EntityTable.N_ENTITIES
        bcc     Loop


    // Process the entities
    // --------------------
    GameLoop:
        jsr     MetaSprite.Render.InitLoop

        jsr     Entities.ProcessGameLoop
        jsr     Entities.RenderLoop

        jsr     MetaSprite.Render.EndLoop

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     EndGameLoop

        bra     GameLoop


EndGameLoop:
    jsr     Test.ResetPpuState

    sec
    rts

Fail:
    clc
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

