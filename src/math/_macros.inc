// math/_macros.inc
// ================
//
// Macros for simple mathematical operations.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Math {

// Negate A (8 bit A)
macro neg.b() {
    assert8a()

    eor.b   #0xff
    inc
}


// Negate A (16 bit A)
macro neg.w() {
    assert16a()

    eor.w   #0xffff
    inc
}


// Negates the 32 bit value in XY
//
// REQUIRES: 16 bit Index, 16 bit A
// MODIFIES: A, X, Y
macro Negate32xy() {
    assert16a()
    assert16i()

    tya
    eor.w   #0xffff
    clc
    adc.w   #1
    tay
    txa
    eor.w   #0xffff
    adc.w   #0
    tax
}


// Negates the 32 bit variable in DP
//
// REQUIRES: 16 bit A
// MODIFIES: A
macro Negate32.b(var) {
    assert16a()

    lda.b   {var}
    eor.w   #0xffff
    clc
    adc.w   #1
    sta.b   {var}
    lda.b   {var} + 2
    eor.w   #0xffff
    adc.w   #0
    sta.b   {var} + 2
}


// Negates the 32 bit variable
//
// REQUIRES: 16 bit A
// MODIFIES: A
macro Negate32.w(var) {
    assert16a()

    lda.w   {var}
    eor.w   #0xffff
    clc
    adc.w   #1
    sta.w   {var}
    lda.w   2 + {var}
    eor.w   #0xffff
    adc.w   #0
    sta.w   2 + {var}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

