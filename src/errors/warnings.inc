// errors/warning.inc
// ==================
//
// Warnings forcibly change the text color to notify the user that a
// non-fatal error has occurred.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


scope Warnings {

// The current value of the warning buffer.
// Will be uploaded to CGRAM if MSB is clear.
// If MSB is set then there is no error.
//
// (word - snes color)
allocate(current, shadow, 2)

// The CGRAM color position to use for notifying the user.
constant CGRAM_POS(1)


// Issue a warning.
//
// PARAM: id - warning identifier
//
// REQUIRES: 16 bit A or X
//
// The identifier needs to have a registered color,
// colors can be registered with the `Warnings.Register` macro.
macro Warn(id) {
    if __CPU__.aSize == 16 {
        lda.w   #__WARNINGS__.ids.{id}
        jsr     Warnings.Warn_a16

    } else if __CPU__.iSize == 16 {
        ldx.w   #__WARNINGS__.ids.{id}
        jsr     Warnings.Warn_i16
    } else {
        error "Expected 16 bit A or 16 bit Index"
    }
}


// Registers a warning identifier with a given color.
//
// Assembly will be aborted if two warnings register similar colors.
//
// This macro MUST BE called in the root scope.
//
// PARAM: id - warning identifier
// PARAM: r,g,b - the RGB values of the warning
macro Register(id, b, g, r) {
    evaluate b({b} & 31)
    evaluate g({g} & 31)
    evaluate r({r} & 31)

    constant __WARNINGS__.ids.{id}(({b} << 10) | ({g} << 5) | {r})


    // Detect similar warning colors

    evaluate lb(({b} + %100) >> 3)
    evaluate lg(({g} + %100) >> 3)
    evaluate lr(({r} + %100) >> 3)

    evaluate lb({lb} > 3 ? 3 : {lb})
    evaluate lg({lg} > 3 ? 3 : {lg})
    evaluate lr({lr} > 3 ? 3 : {lr})

    evaluate lc(({lb} << 4) | ({lg} << 2) | {lr})

    if {lc} == 0 {
        error "warningId `{id}` cannot be black"
    } else if {lc} == %111111 {
        error "warningId `{id}` cannot be white"
    } else if {lb} == {lg} && {lb} == {lr} {
        error "warningId `{id}` cannot be grey"
    }

    if {defined __WARNINGS__.colors.{lc}} {
        warning "Cannot register warning color {id}: too similar to {__WARNINGS__.colors.{lc}}"
    } else {
        global define __WARNINGS__.colors.{lc}({id})
    }
}


// Disables the warning notification
//
// This macro MUST BE called before enabling VBlank.
// This macro SHOULD BE called when blanking the screen.
//
// REQUIRES: 8 bit A, DB access shadow
macro Reset() {
    assert8a()

    lda.b   #0xff
    sta.w   Warnings.current + 1
}

// Disables the warning notification
//
// This macro MUST BE called before enabling VBlank.
// This macro SHOULD BE called when blanking the screen.
//
// REQUIRES: 16 bit A, DB access shadow
macro Reset_a16() {
    assert16a()

    lda.w   #0xffff
    sta.w   Warnings.current
}


// Issue a warning
//
// REQUIRES: V-Blank active
// REQUIRES: 16 bit X
//
// INPUT: X = warning color.
au()
iu()
code()
scope Warn_i16: {
    php
    rep #$30
a16()
i16()
    txa
    bra     Warn_a16.SetCurrent
}

// Issue a warning
//
// REQUIRES: V-Blank active
// REQUIRES: 16 bit A
//
// INPUT: A = warning color.
au()
iu()
code()
scope Warn_a16: {
    php

    rep #$30
a16()
a16()

SetCurrent:
    // allow Warnings to be caught by bsnes-plus
    wdm #0

    and.w   #0x7fff
    sta.l   current

    plp
    rts
}


// VBlank code.
//
// Sets CGRAM color 1 to Waning.current if valid
//
// MUST BE CALLED AFTER Dma.VBlank
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    scope Warnings: {
        ldy.w   Warnings.current + 1
        bmi     Skip
            ldx.b   #Warnings.CGRAM_POS
            stx.w   CGADD

            ldx.w   Warnings.current
            stx.w   CGDATA
            sty.w   CGDATA
    Skip:
    }
}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

