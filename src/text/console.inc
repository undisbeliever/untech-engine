// console.inc
// ===========
//
// A *simple* text tile console
//

scope Text {
scope Console {
    constant BUFFER_WIDTH(32)
    constant BUFFER_HEIGHT(28)

    constant MARGIN_LEFT(3)
    constant MARGIN_TOP(2)

    constant MARGIN_RIGHT(BUFFER_WIDTH - MARGIN_LEFT)
    constant MARGIN_BOTTOM(BUFFER_HEIGHT - MARGIN_TOP)

    constant TEXT_WIDTH(MARGIN_RIGHT - MARGIN_LEFT)
    constant TEXT_HEIGHT(MARGIN_BOTTOM - MARGIN_TOP)


// Buffer
// Each byte in the buffer represents a single character
constant buffer(Text.buffer)

// Location in the buffer
// (word dp)
constant bufferPos(Text.vars)

// End position of the buffer for current event
// (word dp)
constant bufferEndLinePos(Text.vars + 2)

assert(Text.vars >= 4)


// Sets the console cursor to a fixed location
//
// REQUIRES: 16 bit A, DB access shadow
macro SetCursor(evaluate x, evaluate y) {
    assert16a()
    assert({x} >= 0 && {x} < Text.Console.TEXT_WIDTH)
    assert({y} >= 0 && {y} < Text.Console.TEXT_HEIGHT)

    evaluate r((Text.Console.MARGIN_TOP + {y}) * Text.Console.BUFFER_WIDTH)

    lda.w   #{r} + Text.Console.MARGIN_LEFT + {x}
    sta.w   Text.Console.bufferPos

    lda.w   #{r} + Text.Console.MARGIN_RIGHT
    sta.w   Text.Console.bufferEndLinePos
}



// Initialize the console and copy font to VRAM.
//
// NOTE: This does not setup the Tilemap Addresses or
//       size registers
//
// DMA: Uses DMA channel 0
// REQUIRES: 8 bit A, 16 bit Index, DB access registers
// REQUIRES: screen blank
code()
a8()
i16()
scope Init: {
    // Transfer the font to the VRAM

    if Text.Font.FixedTiles.bitDepth == 2 {
        ldx.w   #VRAM_CONSOLE_TILES_WADDR
        stx.w   VMADD

        Dma.ForceBlank.ToVram(Text.Font.FixedTiles)

    } else {
        // ::TODO 1bpp bit depth::

        error "Invalid Text.Font.FixedTiles bitDepth"
    }

    // Reset padding character
    lda.b   #Text.Font.SPACE
    sta.l   Text.paddingChar

    // Clear the vram tilemap

    ldx.w   #VRAM_CONSOLE_MAP_WADDR
    ldy.w   #32 * 32 * 2
    jsr     Dma.ForceBlank.ClearVram


    ldx.w   #VRAM_CONSOLE_MAP_WADDR
    stx.w   Text.bufferVramWaddr

End:
    // CleanBuffer
}



// Clear the Buffer
// REQUIRES: 8 bit A, 16 bit Index
code()
// P & DP & DB unknown
scope ClearBuffer: {
    assert(pc() == Init.End)

    php

    rep     #$30
a16()
i16()
    lda.w   #0
    sta.l   buffer

    // Clear the buffer
    lda.w   #0
    ldx.w   #0
    ldy.w   #32 * 32 / 2
    Loop:
        sta.l   buffer,x
        inx
        inx
        dey
        bne     Loop


    SetCursor(0, 0)


    sep     #$20
a8()
    lda.b   #1
    sta.l   bufferDirty

    plp
    rts
}


// Prints an 8 bit hex address to the buffer
//
// INPUT: A = hex address to print
code()
// P & DB unknown
scope PrintHex8A: {
    php

    rep     #$30
a16()
    sta.l   tmpString
    ldy.w   #1
    bra     PrintHexString._AfterPhp
}


// Prints an 16 bit hex address to the buffer
//
// INPUT: A = hex address to print
// REQUIRES: 16 bit A
code()
// P & DB unknown
scope PrintHex16A: {
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
code()
// P & DB unknown
scope PrintHex16Y: {
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
// INPUT: XY = hex address to print
code()
// P & DB unknown
scope PrintHex32XY: {
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
code()
i16()
// P & DB unknown
scope PrintFarAddr: {
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
code()
a8()
i16()
scope PrintHexString: {
    php
_AfterPhp:
    phb

    sep     #$20
a8()
    lda.b   #0x7e
    pha
    plb
// DB = 0x7e


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
// REQUIRES: 16 bit Index
//
// INPUT: Y - uint16 number to print
code()
i16()
// P, DP, DB unknown
scope PrintU16Y: {
    php
    phb
    phd

    sep     #$20
a8()
    lda.b   #0x7e
    pha
    plb

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U16Y_ToString
}


// Prints an unsigned 16 bit padded integer to buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: Y - uint16 number to print
//        A - number of digits to print
code()
i16()
// P, DP, DB unknown
scope PrintPaddedU16Y: {
    php
    phb
    phd

    sep     #$20
a8()
    pea     0x7e7e
    plb
    plb

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U16Y_ToPaddedString
}

// Prints an unsigned 32 bit integer to buffer
//
// REQUIRES: 32 bit Index
//
// INPUT: XY - uint32 number to print
code()
i16()
// P, DP, DB unknown
scope PrintU32XY: {
    php
    phb
    phd

    sep     #$20
a8()
    lda.b   #0x7e
    pha
    plb

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U32XY_ToString
}


// Prints an unsigned 32 bit padded integer to buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: XY - uint16 number to print
//        A - number of digits to print
code()
i16()
// P, DP, DB unknown
scope PrintPaddedU32XY: {
    php
    phb
    phd

    sep     #$20
a8()
    pea     0x7e7e
    plb
    plb

    pea     PrintString.AfterSetBank - 1
    jmp     Text.String.U32XY_ToPaddedString
}


// Prints a string to the buffer
//
// REQUIRES: 16 bit Index
//
// INPUT: A:X = string address
code()
i16()
// DP & DB unknown
scope PrintString: {
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

    Loop:
        lda.w   0,y
        beq     EndLoop

        sta.l   buffer,x

        iny

        inx
        cpx.b   bufferEndLinePos
        bcc     Loop
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
code()
// P & DP & DB unknown
scope NewLine: {
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

    clc
    adc.w   #MARGIN_RIGHT - MARGIN_LEFT
    sta.l   bufferEndLinePos


    // Empty the next line

    assert(BUFFER_WIDTH % 2 == 0)

    lda.w   #0
    ldy.w   #BUFFER_WIDTH / 2
    ldx.w   bufferPos

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

// vim: ft=asm ts=4 sw=4 et:

