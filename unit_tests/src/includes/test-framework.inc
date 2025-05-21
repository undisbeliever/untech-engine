// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

code()

// Ensure test table block is in the same bank as the code block
assert((__MEMORY__.romBlocks.testTableBlock.start >> 16) == (__MEMORY__.romBlocks.code.start >> 16))

// ::TODO display failed test name on breakpoint::


variable __TESTS__.nTests = 0
variable __TESTS__.testsFinalized = 0

namespace Test {

constant TESTING_COLOR = 0x7fff
constant SUCCESS_COLOR = 22 << Palette.green.shift
constant FAILURE_COLOR = 20 << Palette.red.shift

constant TMP_SIZE = 32
allocate(dpTmp, dp, TMP_SIZE)

allocate(tmp, lowram, TMP_SIZE)


// TABLE LOCATIONS
// ===============
rodata(testTableBlock)
AddrTable:

rodata(testNameBlock)
// A NULL separated list of strings for each test
TestNameList:

// MEMORY ALLOCATIONS
// ==================

allocate(testId, lowram, 2)
allocate(currentTestName, lowram, 2)

// MACROS
// ======

// Adds a test to the testName and testTable blocks
//
// The test code:
//  Starts with:
//    * 16 bit A and Index
//    * DB = $7E
//    * DP = 0
//
//  The code must return by:
//      * return with carry set on success.
//      * either jump to `Test.fail` or return with carry clear on failure.
//
//  The code can change, the register size, DP or DB without worry.
//
// Parameters:
//  {name} - the name of the test
//  pc()   - the address of the test
macro add(name) {
    if __TESTS__.testsFinalized {
        error "Test table is finalized, cannot add any more tests"
    }

    evaluate loc = pc()

    pushBlock()

    rodata(testTableBlock)
        dw  {loc}

    rodata(testNameBlock)
        db  {name}, 0

    popBlock()

    __TESTS__.nTests = __TESTS__.nTests + 1
}

// Finalizes the test table
//
// MUST be called
inline finalizeTable() {
    assert(__TESTS__.testsFinalized == 0)

    constant N_TESTS = __TESTS__.nTests
    __TESTS__.testsFinalized = 1
}



// CODE
// ====

// Process all of the tests in the table one by one
//
// REQUIRES: None
au()
iu()
code()
function ProcessTests {
    jsr     ResetPpuState

    rep     #$30
a16()
i16()
    lda.w   #0
    sta.w   testId
    tcd

    ldx.w   #TestNameList
    stx.w   currentTestName

    Loop:
        // ::TODO print test number

        jsr     PrintCurrentTestName

        Warnings.Reset_a16()

        lda.w   #0
        tcd

        // corrupt utDpTmp values

        jsr     _DirtyUtDpTmp

        ldx.w   #TMP_SIZE - 2
        -
            dec
            sta.b   Test.dpTmp,x
            dec
            sta.w   Test.tmp,x

            dex
            dex
            bpl     -

        // do test

        lda.w   testId
        asl
        tax

        php
        phb

        jsr     (AddrTable,x)
        bcs     +
            jmp     Fail
        +

        rep     #$30
    a16()
        // Run at full speed unless a button is held down
        lda.w   Controller.Joy1.current
        beq     NoButtonsPressed
            jsl     WaitFrame__far
    NoButtonsPressed:


        // Clears the old test name
        Text.Console.SetCursor(0, 1)
        Text.Console.PrintConstString("\n\n\n")

        sep     #$20
    a8()
        // Increment currentTestName
        ldx.w   currentTestName
        dex

        -
            inx
            lda.l   TestNameList & 0xff0000,x
            bne     -

        inx
        stx.w   currentTestName


        // restore screen brightness (if necessary)
        jsr     EnableDisplay_Full

        plb
        plp

    a16()

        lda.w   testId
        inc
        sta.w   testId

        cmp.w   #N_TESTS
        bcs     +
            jmp     Loop
    +


AllTestsPassed:
    jsr     ResetPpuState

    rep     #$30
    sep     #$20
a8()
i16()
    // Change text color to green

    lda.b   #REGISTER_DB
    pha
    plb

    jsl     ForceBlank__far

    stz.w   CGADD

    lda.b   #SUCCESS_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #SUCCESS_COLOR >> 8
    sta.w   CGDATA


    jsr     Text.Console.NewLine

    ldy.w   testId
    jsr     Text.Console.PrintU16Y

    Text.Console.PrintConstString(" tests passed")

    jsr     EnableDisplay_Full


    // Don't use STP, it causes some versions of snes9x to stall
-
    jsl     WaitFrame__far
    bra     -
}


// A test failed.
// The test can jump to this code at any time
//
// REQUIRES: None
au()
iu()
code()
function Fail {
    jsr     ResetPpuState

    rep     #$30
    sep     #$20
a8()
i16()
    lda.b   #REGISTER_DB
    pha
    plb

    jsl     ForceBlank__far

    // Show failure color

    stz.w   CGADD

    lda.b   #FAILURE_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #FAILURE_COLOR >> 8
    sta.w   CGDATA

    // text color (white)
    lda.b   #0xff
    sta.w   CGDATA
    sta.w   CGDATA


    // Print Failed message
    // --------------------

    Text.Console.PrintConstString(" FAILED\n\n\n")

    // Print Address of failed test
    rep     #$30
a16()
    lda.w   testId
    asl
    tax
    lda.l   AddrTable,x
    tax
    lda.w   #AddrTable >> 16
    jsr     Text.Console.PrintFarAddr

    jsr     Text.Console.NewLine

    // Print Test name
    sep     #$20
a8()
    ldx.w   currentTestName
    lda.b   #TestNameList >> 16
    jsr     Text.Console.PrintString

    jsr     EnableDisplay_Full

    // Don't use STP, it causes some versions of snes9x to stall
-
    jsl     WaitFrame__far
    bra     -
}



au()
iu()
code()
function PrintCurrentTestName {
    php

    rep     #$30
a16()
i16()
    Text.Console.SetCursor(0, 2)

    ldx.w   currentTestName
    lda.w   #TestNameList >> 16
    jsr     Text.Console.PrintString

    plp
    rts
}



// Resets the PPU registers to initial state
//
// All tests that change PPU registers MUST call this routine upon clean-up.
//
// REQUIRES: None
//
// OUTPUT: Carry always set
au()
iu()
code()
function ResetPpuState {
    php
    rep     #$30
a16()
i16()
    pea     0x7e7e
    plb
    plb
// DB = 0x7e

    Warnings.Reset_a16()

    // Setup Screen

    lda.w   #Project.Scenes.UnitTest
    jsr     Resources.LoadScene

    jsr     Resources.TransferToPpu


    pea     (0x7e << 8) | REGISTER_DB
    plb
// DB = REGISTER_DB

    sep     #$20
a8()

    // set BG color
    stz.w   CGADD

    lda.b   #TESTING_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #TESTING_COLOR >> 8
    sta.w   CGDATA

    // text color (black)
    stz.w   CGDATA
    stz.w   CGDATA


    Text.Console.PrintConstString("UnTech Unit Tests:")

    jsr     EnableDisplay_Full


    plb
// DB = 0x7e


    plp
    sec
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

