
// Ensure test table block is in the same bank as the code block
assert(({::romBlocks.testTableBlock.start} >> 16) == ({::romBlocks.code.start} >> 16))

// ::TODO display failed test name on breakpoint::


// Test.ProcessTests is the main function
constant Main(Test.ProcessTests)

evaluate ::nTests(0)

scope Test {

constant TESTING_COLOR((22 << Palette.green.shift) | (20 << Palette.red.shift))
constant SUCCESS_COLOR(22 << Palette.green.shift)
constant FAILURE_COLOR(20 << Palette.red.shift)

allocate(dpTmp, dp, 32)

allocate(tmp, shadow, 32)


// TABLE LOCATIONS
// ===============
rodata(testTableBlock)
AddrTable:

rodata(testTableBlock)
NameTable:

// MEMORY ALLOCATIONS
// ==================

allocate(testId, shadow, 2)

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
    if {defined ::testsFinalized} {
        error "Test table is finalized, cannot add any more tests"
    }

    evaluate loc(pc())

    pushBlock()

    rodata(testTableBlock)
        dw  {loc}

    rodata(testNameBlock)
        db  "{name}", 0

    popBlock()

    global evaluate ::nTests({::nTests} + 1)
}

// Finalizes the test table
//
// MUST be called
macro finalizeTable() {
    constant N_TESTS({::nTests})
    global define ::testsFinalized(1)
}



// CODE
// ====

// Process all of the tests in the table one by one
code()
scope ProcessTests: {
    sep     #$20
a8()
    pea     (0x7e << 8) | REGISTER_DB
    plb

// DB = REGISTER_DB
    jsr     ResetRegisters

    scope {
        // set BG color
        stz.w   CGADD

        lda.b   #TESTING_COLOR & 0xff
        sta.w   CGDATA

        lda.b   #TESTING_COLOR >> 8
        sta.w   CGDATA

        lda.b   #15
        sta.w   INIDISP
    }

    plb
// DB = 0x7e

    rep     #$30
a16()
i16()
    lda.w   #0
    tcd

    scope Loop: {
        sta.w   testId

        // ::TODO display test Id::
        // ::TODO add test name Id::

        jsr     EnableVBlank

        lda.w   testId
        asl
        tax

        php
        phd
        phb

        jsr     (AddrTable,x)
        bcc     Fail

        // restore screen brightness
        sep     #$20
    a8()
        lda.b   #15
        sta.l   INIDISP

        plb
        pld
        plp

    a16()

        lda.w   testId
        inc
        cmp.w   #N_TESTS
        bcc     Loop
    }


AllTestsPassed:
    rep     #$30
    sep     #$20
a8()
i16()
    lda.b   #REGISTER_DB
    pha
    plb

    jsr     ResetRegisters

    stz.w   CGADD

    lda.b   #SUCCESS_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #SUCCESS_COLOR >> 8
    sta.w   CGDATA

    lda.b   #15
    sta.w   INIDISP

    // Don't use STP, it causes some versions of snes9x to stall
-
    wai
    bra     -
}


// A test failed.
// The test can jump to this code at any time
//
// Currently shows a red screen
//
// ::SHOULDO display error message::
// ::TODO display failed test name::
// ::TODO display failed test message::
code()
scope Fail: {
    rep     #$30
    sep     #$20
a8()
i16()
    lda.b   #REGISTER_DB
    pha
    plb

    jsr     ResetRegisters

    // Show failure color

    stz.w   CGADD

    lda.b   #FAILURE_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #FAILURE_COLOR >> 8
    sta.w   CGDATA

    // ::TODO display failed test name::

    lda.b   #15
    sta.w   INIDISP

    // Don't use STP, it causes some versions of snes9x to stall
-
    wai
    bra     -
}
}

// vim: ft=asm ts=4 sw=4 et:

