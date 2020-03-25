// text/console.inc
// ================
//
// A *simple* text tile console.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Text {
namespace Console {
    constant BUFFER_WIDTH = 32
    constant BUFFER_HEIGHT = 28

    constant MARGIN_LEFT = 2
    constant MARGIN_TOP = 2

    constant MARGIN_RIGHT = BUFFER_WIDTH - MARGIN_LEFT
    constant MARGIN_BOTTOM = BUFFER_HEIGHT - MARGIN_TOP

    constant TEXT_WIDTH = MARGIN_RIGHT - MARGIN_LEFT
    constant TEXT_HEIGHT = MARGIN_BOTTOM - MARGIN_TOP



// Buffer
// Each byte in the buffer represents a single character
constant buffer = Text.buffer
constant BUFFER_BANK = buffer >> 16

// Location in the buffer
// (word dp)
constant bufferPos = Text.vars

// End position of the buffer for current event
// (word dp)
constant bufferEndLinePos = Text.vars + 2

assert(Text.vars.size >= 4)


// The ROM data block to store constant strings in
// SEE: PrintConstString
if !{defined Text.Console.StringBlock} {
    define StringBlock = rom0
}

// Prints a constant string to the console.
//
// The string will be inserted into the ROM Block inside the variable
// StringBlock.
//
// REQUIRES: 16 bit Index
macro PrintConstString(s) {
    assert16i()

    pushBlock()
        rodata({Text.Console.StringBlock})
        String{#}:
            db  {s}, 0
    popBlock()

    ldx.w   #String{#}
    if __CPU__.aSize == 8 {
        lda.b   #String{#} >> 16
    } else if __CPU__.aSize == 16 {
        lda.w   #String{#} >> 16
    } else {
        error "Unknown A size"
    }
    jsr     Text.Console.PrintString
}


// Prints a string from a label to the console.
//
// REQUIRES: 16 bit Index
macro PrintString(label) {
    assert16i()

    ldx.w   #{label}
    if __CPU__.aSize == 8 {
        lda.b   #({label}) >> 16
    } else if __CPU__.aSize == 16 {
        lda.w   #({label}) >> 16
    } else {
        error "Unknown A size"
    }
    jsr     Text.Console.PrintString
}


// Sets the console cursor to a fixed location
//
// REQUIRES: 16 bit Index, DB access shadow
macro SetCursor(evaluate x, evaluate y) {
    assert16i()
    assert({x} >= 0 && {x} < Text.Console.TEXT_WIDTH)
    assert({y} >= 0 && {y} < Text.Console.TEXT_HEIGHT)

    evaluate r = (Text.Console.MARGIN_TOP + {y}) * Text.Console.BUFFER_WIDTH

    ldx.w   #{r} + Text.Console.MARGIN_LEFT + {x}
    stx.w   Text.Console.bufferPos

    ldx.w   #{r} + Text.Console.MARGIN_RIGHT
    stx.w   Text.Console.bufferEndLinePos
}



// Transfer the font to VRAM and clear the VRAM tilemap
//
// REQUIRES: 16 bit A, 16 bit Index, DB access registers
a16()
i16()
macro Init() {
    assert8a()
    assert16i()

    stz.w   bufferDirty

    ldx.w   #0xffff
    stx.w   Text.tilesVramWaddr
    stx.w   Text.tilemapVramWaddr


    // Reset padding character
    lda.b   #Text.Font.SPACE
    sta.l   Text.paddingChar

    rts
}



// Transfer the font and buffer to VRAM if the text console is enabled.
//
// NOTE: This does not setup the Tilemap Addresses or
//       size registers
//
// MUST ONLY be called by the Resources subsystem
//
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
// REQUIRES: Resources Subsystem initialized with a scene layout set
macro _TransferToPpu() {
    assert8a()
    assert16i()

    assert(64 * 1024 / 2 <= 0x8000)      // if negative then text console is unused
    ldx.w   Text.tilesVramWaddr
    bmi     NotEnabled

        // Transfer the font to the VRAM
        stx.w   VMADD

        if Text.Font.FixedTiles.bitDepth == 2 {
            Dma.ForceBlank.ToVram(Text.Font.FixedTiles)

        } else if Text.Font.FixedTiles.bitDepth == 1 {
            Dma.ForceBlank.ToVramL(Text.Font.FixedTiles)

            ldx.w   Text.tilesVramWaddr
            stx.w   VMADD

            ldy.w   #Text.Font.FixedTiles.size
            jsr     Dma.ForceBlank.ClearVramH

        } else {
            error "Invalid Text.Font.FixedTiles bitDepth"
        }


        // Set the tilemap palette to 0 and priority to high
        ldx.w   Text.tilemapVramWaddr
        ldy.w   #Tilemap.order
        jsr     Dma.ForceBlank.FillTilemap

NotEnabled:
}


// Enables the text buffer
//
// MUST ONLY be called by the Resources subsystem
//
// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
//
// INPUT: Y = bgIndex of the background used by the text subsystem
a16()
i16()
code()
function _EnableTextBuffer {
    tya
    jsr     Resources.SceneLayout.CalcTileAndMapVramWaddr
    sta.w   tilesVramWaddr
    sty.w   tilemapVramWaddr

FallThrough:
}


// Clear the Buffer
// REQUIRES: DB access shadow
au()
iu()
code()
// P & DP & DB unknown
function ClearBuffer {
    assert(pc() == _EnableTextBuffer.FallThrough)

    php

    rep     #$30
a16()
i16()
    // Clear the buffer
    lda.w   #0
    ldx.w   #buffer.size - 2
    Loop:
        sta.l   buffer,x
        dex
        dex
        bpl     Loop


    SetCursor(0, 0)


    sep     #$20
a8()
    lda.b   #1
    sta.w   bufferDirty

    plp
    rts
}


// Prints an 8 bit hex address to the buffer
//
// INPUT: A = hex address to print
au()
iu()
code()
// P & DB unknown
function PrintHex8A {
    php

    rep     #$30
a16()
    sta.l   tmpString
    ldy.w   #1
    bra     PrintHexString._AfterPhp
}


// Prints an 16 bit hex address to the buffer
//
// INPUT: C = hex address to print
au()
iu()
code()
// P & DB unknown
function PrintHex16A {
    php

    rep     #$30
a16()
    sta.l   tmpString

    ldy.w   #2
    bra     PrintHexString._AfterPhp
}


// Prints an 16 bit hex address to the buffer
//
// INPUT: Y = hex address to print
au()
iu()
code()
// P & DB unknown
function PrintHex16Y {
    php

    rep     #$30
a16()
    tya
    sta.l   tmpString

    ldy.w   #2
    bra     PrintHexString._AfterPhp
}


// Prints an 32 bit hex address to the buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: XY = hex address to print
au()
i16()
code()
// P & DB unknown
function PrintHex32XY {
    php

    rep     #$30
a16()
    tya
    sta.l   tmpString

    txa
    sta.l   tmpString + 2

    ldy.w   #4
    bra     PrintHexString._AfterPhp
}


// Prints a 24 bit hex address to the buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: A:X = hex address to print
i16()
code()
// P & DB unknown
function PrintFarAddr {
    php

    sta.l   tmpString + 2

    rep     #$30
a16()
    txa
    sta.l   tmpString

    ldy.w   #3
    bra     PrintHexString._AfterPhp
}


// Prints a byte array has a hex string.
//
// Ensures the entire hex string exists on a single line
//
// REQUIRES: 16 bit Index
// INPUT: tmpString - the byte string to print
//        Y number of bytes to print (MUST BE < tmpString.size)
a8()
i16()
code()
function PrintHexString {
    php
_AfterPhp:
    phb

    sep     #$20
a8()
    lda.b   #BUFFER_BANK
    pha
    plb
// DB = BUFFER_BANK


    // Ensure that hex string fits in the current line
    rep     #$31
a16()
    tya
    asl
    // c clear
    adc.w   bufferPos
    cmp.w   bufferEndLinePos
    bcc     +
        phy
        jsr     NewLine
        ply
+

    // Print the string
    sep     #$20
a8()
    // Y = number of bytes
    ldx.w   bufferPos
    Loop:
        assert(Font.ZERO == 1)
        assert(Font.CAPITAL_A == 1 + 10)

        lda.w   tmpString - 1,y
        lsr
        lsr
        lsr
        lsr
        inc
        sta.w   buffer,x
        inx

        lda.w   tmpString - 1,y
        and.b   #0x0f
        inc
        sta.w   buffer,x
        inx

        dey
        bne     Loop

    stx.w   bufferPos
    cpx.w   bufferEndLinePos
    bcc     +
        jsr     NewLine
+

    lda.b   #1
    sta.w   bufferDirty

    plb
    plp
    rts
}


// Prints an unsigned 16 bit integer to buffer
//
// REQUIRES: 16 bit Index, 16 bit A
//
// INPUT: A - uint16 number to print
a16()
i16()
code()
// DP, DB unknown
function PrintU16A {
    tay
FallThrough:
}

// Prints an unsigned 16 bit integer to buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: Y - uint16 number to print
au()
i16()
code()
// P, DP, DB unknown
function PrintU16Y {
    assert(pc() == PrintU16A.FallThrough)

    php
    phb
    phd

    sep     #$20
a8()
    lda.b   #BUFFER_BANK
    pha
    plb
// DB = BUFFER_BANK

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U16Y_ToString
}


// Prints an unsigned 16 bit padded integer to buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: Y - uint16 number to print
//        A - number of digits to print
au()
i16()
code()
// P, DP, DB unknown
function PrintPaddedU16Y {
    php
    phb
    phd

    // Must not modify A
    sep     #$20
a8()
    pea     BUFFER_BANK | (BUFFER_BANK << 8)
    plb
    plb
// DB = BUFFER_BANK

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U16Y_ToPaddedString
}

// Prints an unsigned 32 bit integer to buffer
//
// REQUIRES: 32 bit Index
//
// INPUT: XY - uint32 number to print
au()
i16()
code()
// P, DP, DB unknown
function PrintU32XY {
    php
    phb
    phd

    sep     #$20
a8()
    lda.b   #BUFFER_BANK
    pha
    plb
// DB = BUFFER_BANK

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U32XY_ToString
}


// Prints an unsigned 32 bit padded integer to buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: XY - uint16 number to print
//        A - number of digits to print
au()
i16()
code()
// P, DP, DB unknown
function PrintPaddedU32XY {
    php
    phb
    phd

    // Must not modify A
    sep     #$20
a8()
    pea     BUFFER_BANK | (BUFFER_BANK << 8)
    plb
    plb
// DB = BUFFER_BANK

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U32XY_ToPaddedString
}


// Prints a string to the buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: A:X = string address
au()
i16()
code()
// DP & DB unknown
function PrintString {
    php
    phb
    phd

    sep     #$20
a8()
    pha
    plb

AfterSetBank:
    txy

    pea     0
    pld

    ldx.b   bufferPos

    dey
    Loop:
        iny
        lda.w   0,y
        beq     EndLoop

        cmp.b   #Text.Font.NEW_LINE
        beq     NL

        sta.l   buffer,x

        inx
        cpx.b   bufferEndLinePos
        bcc     Loop
        NL:
            phy
            jsr     NewLine
            ldx.b   bufferPos
            ply
            bra     Loop
    EndLoop:

    // A = 0
    inc
    sta.b   bufferDirty

    stx.b   bufferPos

    pld
    plb
    plp
    rts
}



// Prints a new line
//
// Shifts the cursor to the start of the next line and clears it.
//
// REQUIRES: None
au()
iu()
code()
// P & DP & DB unknown
function NewLine {
    php

    assert(BUFFER_WIDTH == 32)

    assert(MARGIN_LEFT >= 0)
    assert(MARGIN_LEFT < MARGIN_RIGHT)
    assert(MARGIN_RIGHT <= BUFFER_WIDTH)
    assert(MARGIN_TOP >= 0)
    assert(MARGIN_TOP < MARGIN_BOTTOM)
    assert(MARGIN_BOTTOM <= BUFFER_HEIGHT)

    sep     #$20
a8()
    // Mark buffer as clean
    // Prevents a line glitch if this routine is interrupted by VBlank
    lda.b   #0
    sta.l   bufferDirty


    rep     #$31
a16()
i16()
    lda.l   bufferPos
    and.w   #~(BUFFER_WIDTH - 1)
    // carry clear
    adc.w   #BUFFER_WIDTH + MARGIN_LEFT

    cmp.w   #MARGIN_BOTTOM * BUFFER_WIDTH
    bcc     +
        lda.w   #MARGIN_TOP * BUFFER_WIDTH + MARGIN_LEFT
    +
    sta.l   bufferPos
    tax

    clc
    adc.w   #MARGIN_RIGHT - MARGIN_LEFT
    sta.l   bufferEndLinePos


    // Clear the next line
    // x = bufferPos (start of line)

    assert(BUFFER_WIDTH % 2 == 0)

    lda.w   #0
    ldy.w   #BUFFER_WIDTH / 2

    Loop:
        sta.l   buffer,x
        inx
        inx
        dey
        bne     Loop

    sep     #$20
a8()
    // A = 0
    inc
    sta.l   bufferDirty

    plp
    rts
}


}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

