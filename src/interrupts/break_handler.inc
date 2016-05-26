// break_handler.inc
// =================
//
// The break handler will show a blue screen when BRK is executed
//
// ::TODO replace break handler with error message::

arch snes.cpu

constant BSOD_COLOR(13 << Palette.blue.shift)

code()
scope BreakHandler: {

    // Store registers
    php

    rep     #$30
a16()
i16()
    pha
    phx
    phy
    phd
    phb

    jsr     ResetRegisters

    lda     #$2100
    tcd

    sep     #$20
a8()
    lda.b   #0
    stz.b   CGADD

    lda.b   #BSOD_COLOR & 0xff
    sta.b   CGDATA

    lda.b   #BSOD_COLOR >> 8
    sta.b   CGDATA

    lda.b   #15
    sta.b   INIDISP


    // Don't use STP, it can cause some versions snes9x to freeze
-
    bra     -
}

// vim: ft=asm ts=4 sw=4 et:

