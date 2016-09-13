// reset_handler.inc
// =================
//
// Code to being the SNES into a consistent state upon reset.
//
// This code requires:
//  * A ROM block called `code` that is accessible by DB 0
//  * A RAM block called `stack` that is accessible by DB 0
//
// This code also requires the following optional defines:
//
//  * ROM_SPEED  - the speed of the ROM (slow (default), fast)

arch snes.cpu

code()

// ensure ResetHandler is accessible by interrupt vectors
assert(pc() & 0x3fffff < 0x010000)
assert(pc() & 0x00ffff >= 0x008000)
assert(({::blocks.stack.end} & 0xffff) < 0x2000)
assert(({::blocks.stack.end} >> 16) == 0 || ({::blocks.stack.end} >> 16) == 0x7e)


// Reset the SNES, initializing the registers and clearing all the memory.
scope ResetHandler: {

    jml     reset
reset:

    sei
    clc
    xce             // Switch to native mode

    rep     #$38    // 16 bit A, 16 bit Index, Decimal mode off
a16()
i16()
    ldx.w   #{::blocks.stack.end}
    txs             // Setup stack

    lda.w   #$0000
    tcd             // Reset Direct Page

    sep     #$20
a8()

    // Set Data Bank

    lda.b   #REGISTER_DB
    pha
    plb


    // ROM access time
    if ROM_SPEED.{ROM_SPEED} == 0x10 {
        lda.b   #MEMSEL.fastrom
        sta.b   MEMSEL

    } else {
        stz.b   MEMSEL
    }


    // This will also disable screen and interrupts
    jsr     ClearVramOamAndCgram


    // Clear the WRAM
    // Setup DMA Channel 0 for WRAM
    ldx.w   #0
    stx.w   WMADDL
    stz.w   WMADDH

    ldy.w   #DMAP.direction.toPpu | DMAP.fixed | DMAP.transfer.one | (WMDATA << 8)
    sty.w   DMAP0   // also sets BBAD0

    // X = 0
    stx.w   DAS0

    ldx.w   #SnesHeader.EmptyBytes
    stx.w   A1T0
    stz.w   A1B0

    lda.b   #MDMAEN.dma0
    sta.w   MDMAEN

    // DSA0 is 0, no need to set it again
    sta.w   MDMAEN

    jsr     ResetRegisters

    jml     Main
}


// Clears the VRAM, OAM and CGRAM.
//
// Fills the VRAM and CGRAM with 0 bytes.
// Moves all of the OAM sprites offscreen.
//
// REQUIRES: none
scope ClearVramOamAndCgram: {
    php
    phd
    phb

    pea     (REGISTER_DB << 8) | $20
    plp
    plb
a8()
i16()

    // Disable All interrupts
    // Prevent interruptions
    stz.b   NMITIMEN

    // Disable HDMA
    stz.b   HDMAEN

    lda.b   #INIDISP.force
    sta.b   INIDISP     // Force Screen Blank


    // Fill VRAM with 0 bytes

    ldx.w   #0
    ldy.w   #0
    jsr     Dma.ForceBlank.ClearVram


    // Fill palette with 0 bytes
    ldx.w   #0
    ldy.w   #0
    jsr     Dma.ForceBlank.ClearCgram


    // Move all objects in the OAM offscreen
    rep     #$30
a16()
    lda.w   #$2100
    tcd

    sep #$30
a8()
i8()
    stz.b   OAMADDL
    stz.b   OAMADDH

    ldx.b   #$80
    ldy.b   #240

    lda.b   #128
-
        stx.b   OAMDATA // X
        sty.b   OAMDATA // Y
        stz.b   OAMDATA
        stz.b   OAMDATA // Character + Flags

        dec
        bne     -

    lda.b   #%01010101
    ldx.b   #128 / 4
-
        sta.b   OAMDATA // Data table
        dex
        bne     -

    plb
    pld
    plp
    rts
}


// Reset the registers to a good initial state.
//
// REQUIRES: Requires 6 bytes of stack space
scope ResetRegisters: {
    php
    phd

    rep     #$30
a16()
    lda.w   #0x2100
    tcd

    sep     #$30
a8()
i8()

    // Disable All interrupts
    // Prevent interruptions
    stz.b   NMITIMEN

    // Disable HDMA
    stz.b   HDMAEN

    lda.b   #INIDISP.force
    sta.b   INIDISP     // Force Screen Blank

    stz.b   OBSEL


    // Registers $210d - $2114
    // BG Scroll Locations - Write twice
    ldx.b   #0x2114 - 0x210d
-
        stz.b   0x0d,x
        stz.b   0x0d,x
        dex
        bpl     -

    // Registers $2105 - $210c
    // BG settings and VRAM base addresses
    ldx.b   #$210c - $2105
-
        stz.b   $05,x
        dex
        bpl     -

    // Skipping Mode 7 as any programmer using that mode
    // will set those registers anyway.

    // Increment VRAM by 1 word on reading/writing the high byte of VRAM
    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.b   VMAIN

    // Registers $2123 - $2133
    // Window Settings, BG/OBJ designation, Color Math, Screen Mode
    // All disabled
    ldx.b   #0x2133 - 0x2123
-
        stz.b   0x23,x
        dex
        bpl     -


    // reset all of the DMA registers
    // Registers $4300 - $437f
    rep     #$20
a16()
    lda.w   #0x4300
    tcd

    ldx.b   #0x7e
-
        stz.b   0,x
        inx
        inx
        bpl     -

    pld
    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

