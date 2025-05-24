// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace HdmaCircularWindowTest {

// Tests:
//  * Hdma
//  * Hdma.CircularWindow

constant xPos = Test.dpTmp
constant yPos = Test.dpTmp + 2
constant radius = Test.dpTmp + 4


// Prints the positions on the screen
macro PrintWindowInput() {
    assert8a()
    assert16i()

    constant textXPos = (Text.Console.TEXT_WIDTH - 12) / 2
    constant textYPos = (Text.Console.TEXT_HEIGHT -  6) / 2


    Text.Console.SetCursor(textXPos, textYPos)
    Text.Console.PrintConstString("  xPos: ")

    ldy.b   xPos
    jsr     Text.Console.PrintHex16Y

    Text.Console.SetCursor(textXPos, textYPos + 2)
    Text.Console.PrintConstString("  yPos: ")

    ldy.b   yPos
    jsr     Text.Console.PrintHex16Y

    Text.Console.SetCursor(textXPos, textYPos + 4)
    Text.Console.PrintConstString("radius: ")

    ldy.b   radius
    jsr     Text.Console.PrintHex16Y
}


// The Test
a16()
i16()
code()
Test.add("HDMA Circular Window Test")
function Test {
    lda.w   #Camera.DISPLAY_WIDTH / 2
    sta.b   xPos

    lda.w   #Camera.DISPLAY_HEIGHT / 2
    sta.b   yPos

    lda.w   #90
    sta.b   radius


    // Setup Screen and color window
    // -----------------------------
    sep     #$20
a8()

    pea     (0x7e << 8) | REGISTER_DB
    plb

    jsl     ForceBlank__far

    lda.b   #(WSEL.win1.enable | WSEL.win1.outside) << WOBJSEL.color.shift
    sta.w   WOBJSEL


    lda.b   #CGWSEL.clip.inside | CGWSEL.prevent.outside
    sta.w   CGWSEL

    lda.b   #CGADSUB.color.subtract | CGADSUB.enable.backdrop
    sta.w   CGADSUB


    lda.b   #COLDATA.plane.all | (15 & COLDATA.intensity.mask)
    sta.w   COLDATA

    jsr     EnableDisplay_Full

    plb

    Hdma.Init()

    rep     #$20
a16()
    Hdma.CircularWindow.Init()


    Loop:
        rep     #$20
    a16()

        lda.w   Controller.Joy1.current

        bit.w   #JOY.up
        beq     +
            dec.b   yPos
        +
        bit.w   #JOY.down
        beq     +
            inc.b   yPos
        +
        bit.w   #JOY.left
        beq     +
            dec.b   xPos
        +
        bit.w   #JOY.right
        beq     +
            inc.b   xPos
        +

        sep     #$20
    a8()

        bit.b   #JOYL.l
        beq     +
            dec.b   radius
        +
        bit.b   #JOYL.r
        beq     +
            inc.b   radius
        +


        PrintWindowInput()

        jsr     _DirtyUtDpTmp

        ldx.b   xPos
        ldy.b   yPos
        lda.b   radius
        jsr     Hdma.CircularWindow.DrawCircle


        rep     #$30
    a16()
        jsr     WaitFrame_PrintFreeCycles


        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Hdma.Deactivate()

    jmp     Test.ResetPpuState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

