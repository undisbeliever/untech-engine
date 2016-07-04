
scope InteractiveTests {
scope MetaSpriteInteractiveTest {

allocateTmpWord(tmp)

scope EntityStruct: {
    childstruct(BaseEntity)
        field(frameId, 2)
        field(frameCount, 2)

        field(frameDelay, 2)
    endstruct()

    assert(MetaSprite.status == BaseEntity.MetaSprite.status)
}

rodata(rom0)
scope EntityTable: {
    struct(pc())
        field(frameSetId, 2)
        field(frameDelay, 2)
        field(xPos, 2)
        field(yPos, 2)
    endstruct()

    macro data(frameSet, frameDelay, xPos, yPos) {
        dw  MSFS.{frameSet}
        dw  {frameDelay}
        dw  {xPos}
        dw  {yPos}
    }

    data(Man, 10, 50, 200)
    data(SpikedWalker, 10, 128, 200)
    data(Woman, 10, 256 - 50, 200)

    data(FST_OneTileFixed, 40, 40, 150)
    data(FST_TwoTilesFixed, 40, 256 - 40, 150)

    data(FST_OneTile, 150, 40, 100)
    data(FST_TwoTiles, 150, 256 - 40, 100)

    data(FST_TwoRows, 200, 256 / 2, 45)
}

constant N_ENTITIES((pc() - EntityTable) / EntityTable.size)

// Initialize the test
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
macro Init() {
    assert16a()
    assert16i()

    scope Init: {
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
        jsr     Dma.Init

        MetaSprite.Init()


        // Setup the entities
        // ------------------

        ldx.w   #0
        lda.w   #Entities.entity0
        tcd

        Loop:
            phx
                SetupEntity()

                tdc
                clc
                adc.w   #Entities.ENTITY_SIZE
                tcd
            pla
            clc
            adc.w   #EntityTable.size
            tax

            cpx.w   #EntityTable.size * N_ENTITIES
            bcc     Loop
    }
}


// Initializes and activates an entity
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = Entity address
// INPUT:  X = EntityTable addr
macro SetupEntity() {
    assert16a()
    assert16i()

    scope SetupEntity: {
        lda.w   #0xffff
        sta.b   EntityStruct.frameId

        stz.b   EntityStruct.frameCount

        lda.l   EntityTable.frameDelay,x
        sta.b   EntityStruct.frameDelay


        lda.l   EntityTable.xPos,x
        sta.b   EntityStruct.xPos

        lda.l   EntityTable.yPos,x
        sta.b   EntityStruct.yPos

        lda.l   EntityTable.frameSetId,x

        ldy.w   #0
        MetaSprite._InitEntity()

        jsr     MetaSprite.Activate
    }
}


// Processes a single metasprite.
//
// Increase metasprite frame if when frameCount-- hits 0.
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
// INPUT: DP = entity
macro ProcessMetaspriteFrame() {
    assert16a()
    assert16i()

    scope ProcessMetaspriteFrame: {
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
            sta.w   tmp

            jsr     MetaSprite.SetFrame
            bcc     EndIf

                // frameCount is only reset upon a successful SetFrame
                // so that cases where we run out of DMA time are handled.
                lda.w   tmp
                sta.b   EntityStruct.frameId
                lda.b   EntityStruct.frameDelay
                sta.b   EntityStruct.frameCount
        EndIf:
    a16()

        lda.b   EntityStruct.xPos
        sec
        sbc.w   #INT_MS8_OFFSET
        sta.w   MetaSprite.Render.xPos

        lda.b   EntityStruct.yPos
        sec
        sbc.w   #INT_MS8_OFFSET
        sta.w   MetaSprite.Render.yPos

        jsr     MetaSprite.Render.RenderEntity
    }
}


// Processes a single frame
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7E
macro ProcessFrame() {
    assert16a()
    assert16i()

    scope ProcessFrame: {
        lda.w   #Entities.entity0

        Loop:
            tcd

            ProcessMetaspriteFrame()

            tdc
            clc
            adc.w   #Entities.ENTITY_SIZE

            cmp.w   #Entities.entity0 + Entities.ENTITY_SIZE * N_ENTITIES
            bcc     Loop
    }
}


// The Test
code()
a16()
i16()
Test.add("MetaSprite")  // ::TODO replace with something better::
scope Test: {
    -
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     -

    Init()

    Loop:
        jsr     MetaSprite.Render.InitLoop

            ProcessFrame()

        jsr     MetaSprite.Render.EndLoop

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     End

        jmp     Loop

End:
    jsr     Test.ResetPpuState

    sec
    rts
}

}
}

// vim: ft=asm ts=4 sw=4 et:

