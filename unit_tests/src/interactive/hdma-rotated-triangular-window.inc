// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2020, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace HdmaRotatedTriangularWindowTest {

// Tests:
//  * Hdma.RotatedTriangularWindow

constant angle = Test.dpTmp
constant diameter = Test.dpTmp + 2


// The Test
a16()
i16()
code()
Test.add("HDMA Rotated Triangle Test")
function Test {

    stz.b   angle

    lda.w   #90
    sta.b   diameter


    Text.Console.SetCursor(0, 22)
    Text.Console.PrintConstString("Use L / R to adjust diameter")


    // Setup Screen and color window
    // -----------------------------
    sep     #$20
a8()

    pea     (0x7e << 8) | REGISTER_DB
    plb

    jsr     ForceBlank

    lda.b   #(WSEL.win1.enable | WSEL.win1.outside) << WOBJSEL.color.shift
    sta.w   WOBJSEL


    lda.b   #CGWSEL.clip.outside | CGWSEL.prevent.inside
    sta.w   CGWSEL

    lda.b   #CGADSUB.color.subtract | CGADSUB.enable.backdrop
    sta.w   CGADSUB


    lda.b   #COLDATA.plane.all | (20 & COLDATA.intensity.mask)
    sta.w   COLDATA
    lda.b   #COLDATA.plane.green | (7 & COLDATA.intensity.mask)
    sta.w   COLDATA

    jsr     EnableDisplay_Full

    plb

    Hdma.Init()


    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        lda.w   #Camera.DISPLAY_WIDTH / 2
        sta.b   Hdma.RotatedTriangularWindow.xPos

        lda.w   #Camera.DISPLAY_HEIGHT / 2
        sta.b   Hdma.RotatedTriangularWindow.yPos

        lda.b   angle
        ldy.b   diameter

        jsr     Hdma.RotatedTriangularWindow


        inc.b   angle


        lda.w   Controller.Joy1.current
        bit.w   #JOY.r
        beq     ++
            ldy.b   diameter
            iny
            cpy.w   #255 + 1
            bcc     +
                ldy.w   #255
            +
            sty.b   diameter
        +

        // A = Controller.Joy1.current
        bit.w   #JOY.l
        beq     ++
            ldy.b   diameter
            dey
            cpy.w   #1
            bcs     +
                ldy.w   #1
            +
            sty.b   diameter
        +

        // A = JOY1
        and.w   #JOY.buttons
        beq     Loop

    Hdma.Deactivate()

    jmp     Test.ResetPpuState
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

