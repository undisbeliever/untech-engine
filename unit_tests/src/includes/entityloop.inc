// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace EntityLoop {

// Initialize the Entity Loop
//
// REQUIRES: 16 bit A, 16 bit Index, Autojoy enabled
// MODIFIES: DB = 0x7e
a16()
i16()
code()
function Init {

    // Setup The screen
    // ----------------
    pea     (0x7e << 8) | REGISTER_DB
    plb
// DB = REGISTER_DB

    sep     #$20
a8()
    jsr     ForceBlank

    assert(VRAM_OBJ_WADDR % OBSEL.base.walign == 0)

    lda.b   #(VRAM_OBJ_WADDR / OBSEL.base.walign) | OBSEL.size.s8_16
    sta.w   OBSEL

    lda.b   #TM.bg1 | TM.obj
    sta.w   TM

    jsr     EnableDisplay_Full

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
function Cleanup {
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
function ProcessUntilButtonPress {
    Loop:
        Controller.UpdateControllerState()

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
function ProcessFrame {
    Controller.UpdateControllerState()

    jsr     MetaSprite.Render.InitLoop

    jsr     Entity.ProcessGameLoop
    jsr     Entity.RenderLoop

    jmp     MetaSprite.Render.EndLoop
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

