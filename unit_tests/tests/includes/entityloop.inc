
scope InteractiveTests {
scope EntityLoop {

// Initialize the Entity Loop
//
// REQUIRES: 16 bit A, 16 bit Index, Autojoy enabled
// MODIFIES: DB = 0x7e
a16()
i16()
code()
scope Init: {

    // Wait for button release
    // -----------------------
    WaitButtonRelease:
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     WaitButtonRelease


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
    jsr     Entity.Init

    rts
}


// Cleans up the screen
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
//
// OUTPUT: c set
a16()
i16()
code()
scope Cleanup: {
    jsr     Test.ResetPpuState

    sec
    rts
}


// Process the Entity Loop until a joypad button has been pressed
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
// MODIFIES: DP
//
// OUTPUT: c set
a16()
i16()
code()
scope ProcessUntilButtonPress: {
    Loop:
        jsr     ProcessFrame

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame

        lda.l   JOY1
        and.w   #JOY.buttons
        beq     Loop

    jmp     Cleanup
}


// Processes a single frame of the Entity Loop
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, Init called
// MODIFIES: DP
//
// OUTPUT: z clear if a button has been pressed
a16()
i16()
code()
scope ProcessFrame: {
    jsr     MetaSprite.Render.InitLoop

    jsr     Entity.ProcessGameLoop
    jsr     Entity.RenderLoop

    jmp     MetaSprite.Render.EndLoop
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

