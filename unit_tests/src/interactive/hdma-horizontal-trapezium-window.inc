// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

namespace InteractiveTests {
namespace HdmaHorizontalTrapeziumTest {

// Tests:
//  * Hdma
//  * Hdma.HorizontalTrapezium


constant yPos =         Test.dpTmp + 0
constant height =       Test.dpTmp + 2
constant topLeft =      Test.dpTmp + 4
constant topRight =     Test.dpTmp + 6
constant bottomLeft =   Test.dpTmp + 8
constant bottomRight =  Test.dpTmp + 10

constant cursor =       Test.dpTmp + 12

constant FIRST_VAR = yPos
constant N_VARIABLES =  6


// Prints the positions on the screen
macro PrintWindowInput() {
    assert16a()
    assert16i()

    constant textXPos = 0
    constant textYPos = 4

    macro PrintVar(line, cursorId, name) {
        Text.Console.SetCursor(textXPos, textYPos + {line})

        ldx.b   cursor
        cpx.w   #{cursorId}
        bne     +
            ldx.w   #CursorString
            bra     ++
        +
            ldx.w   #NoCursorString
        +
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        Text.Console.PrintConstString("{name}:")

        Text.Console.SetCursor(textXPos + 15, textYPos + {line})
        ldy.b   {name}
        jsr     Text.Console.PrintHex16Y
    }
    PrintVar(0, 0, yPos)
    PrintVar(1, 1, height)
    PrintVar(3, 2, topLeft)
    PrintVar(4, 3, topRight)
    PrintVar(6, 4, bottomLeft)
    PrintVar(7, 5, bottomRight)
}

rodata(rom0)
CursorString:
    db  " >", 0

NoCursorString:
    db  "  ", 0


// Move the trapezium with the d-pad
macro MoveTrapeziumWithDPad() {
    assert16a()
    assert16i()

    lda.w   Controller.Joy1.current
    tax

    bit.w   #JOY.up
    beq     TestDown
        // Up pressed
        lda.b   yPos
        clc
        adc.b   height
        bpl     +
        cmp.w   #-16
        bcc     TestDown
        +
            // bottom of trapezium > -16
            dec.b   yPos

TestDown:
    txa
    bit.w   #JOY.down
    beq     TestLeft
        // Down pressed
        lda.b   yPos
        bmi     +
        cmp.w   #Camera.DISPLAY_HEIGHT + 16
        bcs     TestLeft
        +
            // top of trapezium < DISPLAY_HEIGHT + 16
            inc.b   yPos

TestLeft:
    txa
    bit.w   #JOY.left
    beq     TestRight
        // Left Pressed
        lda.b   topRight
        sec
        sbc.b   bottomRight
        bvc     +
            eor.w   #0x8000
        +
        bpl     +
            lda.b   bottomRight
            bra     ++
        +
            lda.b   topRight
        +
        // A = max(topRight, bottomRight)
        bpl     +
        cmp.w   #-16
        bcc     TestRight
        +
            // leftmost point of trapezium > -16
            dec.b   topLeft
            dec.b   bottomLeft
            dec.b   topRight
            dec.b   bottomRight

TestRight:
    txa
    bit.w   #JOY.right
    beq     EndIf
        // Right Pressed
        lda.b   topLeft
        sec
        sbc.b   bottomLeft
        bvc     +
            eor.w   #0x8000
        +
        bmi     +
            lda.b   bottomLeft
            bra     ++
        +
            lda.b   topLeft
        +
        // A = min(topLeft, bottomLeft)
        bmi     +
        cmp.w   #Camera.DISPLAY_WIDTH + 16
        bcs     EndIf
        +
            // leftmost point of trapezium < DISPLAY_WIDTH + 16
            inc.b   topLeft
            inc.b   bottomLeft
            inc.b   topRight
            inc.b   bottomRight
EndIf:
}


// Update the variables with the
macro UpdateVariables() {
    assert16a()
    assert16i()

    ldx.b   cursor

    lda.w   Controller.Joy1.pressed
    and.w   #JOY.shoulder
    beq     CursorUnchanged

        and.w   #JOY.r
        beq     LPressed
            // R Pressed
            inx
            cpx.w   #N_VARIABLES + 1
            bcc     +
                ldx.w   #0
            +
            bra     StoreCursor

        LPressed:
            dex
            bpl     +
                ldx.w   #N_VARIABLES
            +
    StoreCursor:
        stx.w   cursor

    CursorUnchanged:

    // X = cursor
    cpx.w   #N_VARIABLES
    bcc     UpdateSingleVariable
        MoveTrapeziumWithDPad()
        bra     EndIf

    UpdateSingleVariable:
        // Update just one variable
        txa
        asl
        tax

        lda.w   Controller.Joy1.current
        and.w   #JOY.left
        beq     TestRight
            lda.b   FIRST_VAR,x
            dec
            bra     ClampValue

    TestRight:
        lda.w   Controller.Joy1.current
        and.w   #JOY.right
        beq     EndIf
            lda.b   FIRST_VAR,x
            inc

        ClampValue:
            // a = bound(a, -255, 511)

            bpl     +
            cmp.w   #-255 + 1
            bcs     ++
                lda.w   #-255
                bra     ++
            +
            cmp.w   #511 + 1
            bcc     +
                lda.w   #511
            +
            sta.b   FIRST_VAR,x
    EndIf:


    // Enforce limits

    ldx.b   cursor

    // height must be between 1 and 255
    lda.b   height
    bmi     +
    bne     ++
    +
        lda.w   #1
    +
    cmp.w   #256
    bcc     +
        lda.w   #255
    +
    sta.b   height

    inline _LimitX(left, right) {
        lda.b   {left}
        sec
        sbc.b   {right}
        bvc     +
            eor.w   #0x8000
        +
        bmi     ++
            // {left} > {right}

            cpx.w   #({left} - FIRST_VAR) / 2
            beq     +
                lda.b   {left}
                sta.b   {right}
                bra     ++
            +
                lda.b   {right}
                sta.b   {left}
        +
    }
    _LimitX(topLeft, topRight)
    _LimitX(bottomLeft, bottomRight)
}


// The Test
a16()
i16()
code()
Test.add("HDMA Horizontal Trapezium")
function Test {
    macro SetVar(name, val) {
        lda.w   #{val}
        sta.b   {name}
    }
    SetVar(yPos,        30)
    SetVar(height,      224 - 30 * 2)
    SetVar(topLeft,     128 - 8)
    SetVar(topRight,    128 + 8)
    SetVar(bottomLeft,  128 - 91)
    SetVar(bottomRight, 128 + 91)

    SetVar(cursor,      N_VARIABLES)


    Text.Console.SetCursor(0, 21)
    Text.Console.PrintString(UnitTestMessage)


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


    lda.b   #COLDATA.plane.all | (24 & COLDATA.intensity.mask)
    sta.w   COLDATA
    lda.b   #COLDATA.plane.blue | (6 & COLDATA.intensity.mask)
    sta.w   COLDATA

    jsr     EnableDisplay_Full

    plb

    Hdma.Init()
    jsr     Hdma.CircularWindow.Init

    rep     #$30
a16()

    Loop:
        jsr     WaitFrame_Print

        Controller.UpdateControllerState()

        UpdateVariables()

        PrintWindowInput()

        macro CopyVar(name) {
            lda.b   {name}
            sta.b   Hdma.HorizontalTrapeziumWindow.{name}
        }
        CopyVar(yPos)
        CopyVar(height)
        CopyVar(topLeft)
        CopyVar(topRight)
        CopyVar(bottomLeft)
        CopyVar(bottomRight)
        jsr     Hdma.HorizontalTrapeziumWindow

        lda.l   JOY1
        and.w   #JOY.buttons
        bne     Pass

        jmp     Loop

Pass:
    Hdma.Deactivate()

    jmp     Test.ResetPpuState
}

rodata(rom0)
UnitTestMessage:
    db  "Use L / R to select variable"
    db  "Use D-PAD to adjust value\n", 0
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

