// interrupts/break_handler.inc
// ============================
//
// The break handler stops CPU execution and displays a blue screen of
// death containing the contents of the CPU registers and the stack.
//
// The blue screen will also contain the identifier string of the brk
// interrupt (see `break` macro) if the `breakIdStrings` ROM block is
// defined.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


architecture wdc65816

namespace BreakHandler {

constant BSOD_BG_COLOR = 13 << Palette.blue.shift
constant BSOD_TEXT_COLOR = 0x7fff

// Code must be in bank 0
code()
assert((pc() & 0x3fffff) >= 0x008000 && (pc() & 0x3fffff) < 0x00ffff)


// Initializes the error text console
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, Registers reset, ForceBlank
macro Init() {
    assert8a()
    assert16i()

    // Setup PPU

    stz.b   BGMODE

    lda.b   #((VRAM_CONSOLE_MAP_WADDR / BGXSC.base.walign) << BGXSC.base.shift) | BGXSC.map.s32x32
    sta.b   BG1SC

    lda.b   #(VRAM_CONSOLE_TILES_WADDR / BG12NBA.walign) << BG12NBA.bg1.shift
    sta.b   BG12NBA

    lda.b   #TM.bg1
    sta.b   TM


    // Load CGRAM

    stz.b   CGADD

    lda.b   #BSOD_BG_COLOR & 0xff
    sta.b   CGDATA

    lda.b   #BSOD_BG_COLOR >> 8
    sta.b   CGDATA

    lda.b   #BSOD_TEXT_COLOR & 0xff
    sta.b   CGDATA

    lda.b   #BSOD_TEXT_COLOR >> 8
    sta.b   CGDATA


    // Load Font

    lda.b   #VMAIN.incrementMode.high | VMAIN.increment.by1
    sta.w   VMAIN

    rep     #$30
a16()
    lda.w   #VRAM_CONSOLE_TILES_WADDR
    sta.b   VMADD

    ldx.w   #0
    LoadFontLoop:
        lda.l   Text.Font.FixedTiles,x
        if Text.Font.FixedTiles.bitDepth == 1 {
            and.w   #0x00ff
        }
        sta.b   VMDATA

        inx
        if Text.Font.FixedTiles.bitDepth == 2 {
            inx
        }
        cpx.w   #Text.Font.FixedTiles.size
        bcc     LoadFontLoop


    // Clear TextMap

    lda.w   #VRAM_CONSOLE_MAP_WADDR
    sta.b   VMADD

    ldx.w   #32 * 32
    ClearConsoleLoop:
        stz.b   VMDATA
        dex
        bne     ClearConsoleLoop


    sep     #$20
a8()

    // set VMAIN to increment +1 low

    assert(VMAIN.incrementMode.low | VMAIN.increment.by1 == 0)
    stz.b   VMAIN
}


// Sets VMADD to the given cursor position
//
// REQUIRES: 16 bit Index, DP = 0x2100
macro SetCursor(evaluate xPos, evaluate yPos) {
    assert16i()
    ldy.w   #VRAM_CONSOLE_MAP_WADDR + {yPos} * 32 + {xPos}
    sty.b   VMADD
}


// Prints the breakIdString from a given Id
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: A - the BRK Id
macro PrintBreakIdString() {
    assert8a()
    assert16i()

    // IdStrings - a concatenation of strings
    storeBlockStart(IdStrings, breakIdStrings)
    storeBlockUsed(IdSrings.size, breakIdStrings)

    sep     #$30
i8()
    ldx.b   #0
    tay
    rep     #$10
i16()
    beq     PrintLoop

    FindStringLoop:
        lda.l   IdStrings,x
        bne     +
            cpx.w   #IdSrings.size
            bcs     StringNotFound

            dey
            beq     StringFound
        +
        inx
        bra     FindStringLoop

StringFound:
    inx

    PrintLoop:
        lda.l   IdStrings,x
        beq     EndLoop

        sta.b   VMDATA

        inx
        bra     PrintLoop

EndLoop:
StringNotFound:
}


// Prints a cstring in the TextBank to VRAM
//
// REQUIRES: DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: X = cstring to print
au()
iu()
code()
function PrintString {
    php
    rep     #$30
    sep     #$20

a16()
i16()
    Loop:
        lda.l   TextBank << 16,x
        beq     EndLoop

        sta.b   VMDATA

        inx
        bra     Loop

EndLoop:
    plp
    rts
}


// Prints a string and then a byte as a hexadecimal to VRAM
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: X = string to print
// INPUT: A = byte to print as hex
a8()
i16()
code()
function PrintStringAndHex8 {
    pha
    jsr     PrintString
    pla
FallThrough:
}

// Prints a byte as a hexadecimal to VRAM
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: A = byte to print
// KEEP: X
a8()
i16()
code()
function PrintHex8 {
    assert(PrintStringAndHex8.FallThrough == pc())

    assert(Text.Font.ZERO == 1)
    assert(Text.Font.CAPITAL_A == 1 + 10)

    tay

    lsr
    lsr
    lsr
    lsr
    inc
    sta.b   VMDATA

    tya
    and.b   #0x0f
    inc
    sta.b   VMDATA

    rts
}


// Prints a string and then a word as a hexadecimal to VRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: X = string to print
// INPUT: A = word to print as hex
a16()
i16()
code()
function PrintStringAndHex16 {
    pha
    jsr     PrintString
    pla
FallThrough:
}

// Prints a word as a hexadecimal to VRAM
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMADD set to cursor position, VMAIN set to increment +1 low
//
// INPUT: A = word to print
a16()
i16()
code()
function PrintHex16 {
    assert(PrintStringAndHex16.FallThrough == pc())

    tax
    sep     #$20
a8()
    xba
    jsr     PrintHex8
    txa
    jsr     PrintHex8

    rep     #$20
a16()
    rts
}


// Print a string at a given location and then a hexadecimal byte from the stack
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMAIN set to increment +1 low
macro Print8BitRegister(evaluate xPos, evaluate yPos, str, evaluate stackOffset) {
    assert8a()
    assert16i()

    SetCursor({xPos}, {yPos})

    ldx.w   #{str}
    lda     {stackOffset},s
    jsr     PrintStringAndHex8
}


// Print a string at a given location and then a hexadecimal word from the stack
//
// REQUIRES: 16 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMAIN set to increment +1 low
macro Print16BitRegister(evaluate xPos, evaluate yPos, str, evaluate stackOffset) {
    assert16a()
    assert16i()

    SetCursor({xPos}, {yPos})

    ldx.w   #{str}
    lda     {stackOffset},s
    jsr     PrintStringAndHex16
}


// Print Breakpoint message and ID to screen
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, DB = 0, ForceBlank
// REQUIRES: VMAIN set to increment +1 low
//
// INPUT: CPU Registers pushed to stack
macro PrintBreakpoint() {
    assert8a()
    assert16i()

    // Break String and Break Id
    SetCursor(2, 3)
    lda     13,s
    pha
    plb

    rep     #$30
a16()
    lda     11,s
    dec
    tax

    sep     #$20
a8()
    lda.w   0,x
    ldx.w   #BreakString

    phk
    plb

    if {defined __MEMORY__.romBlocks.breakIdStrings} {
        pha
        jsr     PrintStringAndHex8

        SetCursor(2, 4)

        pla
        PrintBreakIdString()
    } else {
        jsr     PrintStringAndHex8
    }
}


// Print CPU Register Status
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, ForceBlank
// REQUIRES: VMAIN set to increment +1 low
//
// INPUT: CPU Registers pushed to stack
macro PrintCpuState() {
    assert8a()
    assert16i()

    // PC
    SetCursor(2, 6)

    ldx.w   #PcString
    lda     13,s
    jsr     PrintStringAndHex8

    rep     #$30
a16()
    lda     11,s
    dec
    dec
    jsr     PrintHex16


    // Stack
    SetCursor(3, 8)

    tsa
    clc
    adc.w   #13
    ldx.w   #SString
    jsr     PrintStringAndHex16


    Print16BitRegister( 3, 7, AString, 8)
    Print16BitRegister(13, 7, XString, 6)
    Print16BitRegister(22, 7, YString, 4)

    Print16BitRegister(12, 8, DPString, 2)

    sep     #$20
a8()
    Print8BitRegister(22, 6, PString, 9)
    Print8BitRegister(21, 8, DBString, 1)
}


// Print Stack
//
// REQUIRES: 8 bit A, 16 bit Index, DP = 0x2100, DB = 0, ForceBlank
// REQUIRES: VMAIN set to increment +1 low
//
// INPUT: CPU Registers pushed to stack
macro PrintStack() {
    constant BYTES_PER_LINE = 8
    constant YPOS = 10
    constant YBOTTOM = 3

    constant N_LINES = 28 - YPOS - YBOTTOM
    constant PADDING = 32 - (BYTES_PER_LINE * 3)

    allocate(byteCount, shadow, 1)
    allocate(lineCount, shadow, 1)


    assert8a()
    assert16i()

    SetCursor(PADDING / 2, YPOS)

    rep     #$30
a16()
    tsa
    clc
    adc.w   #14
    tax

    sep     #$20
a8()

    lda.b   #N_LINES
    sta.w   lineCount

    // X = stack at BRK
    PageLoop:
        lda.b   #BYTES_PER_LINE
        sta.w   byteCount

        LineLoop:
            cpx.w   #STACK_BOTTOM + 1
            bcs     Return

            lda.w   0,x
            jsr     PrintHex8

            stz.b   VMDATA

            inx

            dec     byteCount
            bne     LineLoop


        dec.w   lineCount
        beq     Return


        lda.b   #PADDING
        PaddingLoop:
            stz.b   VMDATA
            dec
            bne     PaddingLoop

        bra     PageLoop

Return:
}


// Breakpoint Interrupt Service Routine
au()
iu()
code()
function ISR {
    // Store registers
    rep     #$30
a16()
i16()
    pha
    phx
    phy
    phd
    phb

    // ResetRegisters disables interrupts so I do not have to

    jsr     ResetRegisters

    // DB = 0, DP = $2100
    phk
    plb

    lda     #$2100
    tcd

    sep     #$20
a8()

    Init()

    PrintBreakpoint()
    PrintCpuState()
    PrintStack()

    lda.b   #15
    sta.b   INIDISP


    // Don't use STP, it can cause some versions snes9x to freeze
-
    bra     -
}

rodata(rom0)
constant TextBank = pc() >> 16

BreakString:
    db  "BRK ", 0

PcString:
    db  "PC: ", 0

PString:
    db  "P: ", 0

AString:
    db  "A: ", 0

XString:
    db  "X: ", 0

YString:
    db  "Y: ", 0

SString:
    db  "S: ", 0

DPString:
    db  "DP: ", 0

DBString:
    db  "DB: ", 0
}

constant BreakHandler = BreakHandler.ISR

// vim: ft=bass-65816 ts=4 sw=4 et:

