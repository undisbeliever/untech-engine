
scope InteractiveTests {
scope HdmaCircularWindowTest {

// Tests:
//  * Hdma
//  * Hdma.CircularWindow

constant xPos(Test.dpTmp)
constant yPos(Test.dpTmp + 2)
constant radius(Test.dpTmp + 4)


// Prints the positions on the screen
macro PrintWindowInput() {
    assert8a()
    assert16i()

    constant textXPos((Text.Console.TEXT_WIDTH - 12) / 2)
    constant textYPos((Text.Console.TEXT_HEIGHT -  6) / 2)


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
scope Test: {
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

    lda.b   #INIDISP.force
    sta.w   INIDISP

    lda.b   #(WSEL.win1.enable | WSEL.win1.outside) << WOBJSEL.color.shift
    sta.w   WOBJSEL


    lda.b   #CGWSEL.clip.inside | CGWSEL.prevent.outside
    sta.w   CGWSEL

    lda.b   #CGADSUB.color.subtract | CGADSUB.enable.backdrop
    sta.w   CGADSUB


    lda.b   #COLDATA.plane.all | (15 & COLDATA.intensity.mask)
    sta.w   COLDATA

    lda.b   #15
    sta.w   INIDISP

    plb

    Hdma.Init()
    jsr     Hdma.CircularWindow.Init


    Loop:
        rep     #$20
    a16()

        lda.l   JOY1

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

        // corrupt dpTmp
        ldx.w   #0xffff
        stx.b   dpTmp0
        stx.b   dpTmp1
        stx.b   dpTmp2
        stx.b   dpTmp3
        stx.b   dpTmp4
        stx.b   dpTmp5
        stx.b   dpTmp6
        stx.b   dpTmp7


        ldx.b   xPos
        ldy.b   yPos
        lda.b   radius
        jsr     Hdma.CircularWindow.DrawCircle


        rep     #$30
    a16()
        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame


        lda.l   JOY1
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
