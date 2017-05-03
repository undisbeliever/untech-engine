
code()

// Ensure test table block is in the same bank as the code block
assert(({::romBlocks.testTableBlock.start} >> 16) == ({::romBlocks.code.start} >> 16))

// ::TODO display failed test name on breakpoint::


evaluate ::nTests(0)

scope Test {

constant TESTING_COLOR(0x7fff)
constant SUCCESS_COLOR(22 << Palette.green.shift)
constant FAILURE_COLOR(20 << Palette.red.shift)

allocate(dpTmp, dp, 32)

allocate(tmp, shadow, 32)


// TABLE LOCATIONS
// ===============
rodata(testTableBlock)
AddrTable:

rodata(testNameBlock)
// A NULL separated list of strings for each test
TestNameList:

// MEMORY ALLOCATIONS
// ==================

allocate(testId, shadow, 2)
allocate(currentTestName, shadow, 2)

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
        db  {name}, 0

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
//
// REQUIRES: None
au()
iu()
code()
scope ProcessTests: {
    jsr     ResetPpuState

    rep     #$30
a16()
i16()
    lda.w   #0
    sta.w   testId
    tcd

    ldx.w   #TestNameList
    stx.w   currentTestName

    scope Loop: {
        // ::TODO print test number

        Text.Console.SetCursor(0, 2)

        ldx.w   currentTestName
        lda.w   #TestNameList >> 16
        jsr     Text.Console.PrintString

        jsr     EnableVBlank


        lda.w   testId
        asl
        tax

        lda.w   #0
        tcd

        php
        phb

        jsr     (AddrTable,x)
        bcs     +
            jmp     Fail
        +


        rep     #$30
    a16()
        WaitForButtonRelease:
            lda.l   JOY1
            beq     NoButtonsPressed
                jsr     WaitFrame
                bra     WaitForButtonRelease
        NoButtonsPressed:


        // Clears the old test name
        Text.Console.SetCursor(0, 1)
        Text.Console.PrintConstString("\n\n\n")

        sep     #$20
    a8()
        // Increment currentTestName
        scope {
            ldx.w   currentTestName
            dex

            Loop:
                inx
                lda.l   TestNameList & 0xff0000,x
                bne     Loop

            inx
            stx.w   currentTestName
        }


        // restore screen brightness (if necessary)
        lda.b   #15
        sta.l   INIDISP

        plb
        plp

    a16()

        lda.w   testId
        inc
        sta.w   testId

        cmp.w   #N_TESTS
        bcc     Loop
    }


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

    lda.b   #INIDISP.force
    sta.w   INIDISP

    stz.w   CGADD

    lda.b   #SUCCESS_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #SUCCESS_COLOR >> 8
    sta.w   CGDATA


    jsr     Text.Console.NewLine

    ldy.w   testId
    jsr     Text.Console.PrintU16Y

    Text.Console.PrintConstString(" tests passed")

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
// REQUIRES: None
au()
iu()
code()
scope Fail: {
    jsr     ResetPpuState

    rep     #$30
    sep     #$20
a8()
i16()
    lda.b   #REGISTER_DB
    pha
    plb

    lda.b   #INIDISP.force
    sta.w   INIDISP

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

    lda.b   #15
    sta.w   INIDISP

    // Don't use STP, it causes some versions of snes9x to stall
-
    wai
    bra     -
}


// Resets the PPU registers to initial state
//
// All tests that change PPU registers MUST call this routine upon clean-up
//
// REQUIRES: None
//
// OUTPUT: Carry always set
au()
iu()
code()
scope ResetPpuState: {
    php
    rep     #$30
    sep     #$20
a8()
i16()
    pea     (0x7e << 8) | REGISTER_DB
    plb
// DB = REGISTER_DB

    jsr     ResetRegisters


    // Setup Screen
    //
    // MODE1
    // BG1 - text console buffer

    // ::TODO move elsewhere::
    // ::TODO design how to do this, I'm thinking of a macro that generates for each map layout::

    stz.w   BGMODE
    lda.b   #((VRAM_CONSOLE_MAP_WADDR / BGXSC.base.walign) << BGXSC.base.shift) | BGXSC.map.s32x32
    sta.w   BG1SC

    lda.b   #(VRAM_CONSOLE_TILES_WADDR / BG12NBA.walign) << BG12NBA.bg1.shift
    sta.w   BG12NBA

    lda.b   #TM.bg1
    sta.w   TM


    // set BG color
    stz.w   CGADD

    lda.b   #TESTING_COLOR & 0xff
    sta.w   CGDATA

    lda.b   #TESTING_COLOR >> 8
    sta.w   CGDATA

    // text color (black)
    stz.w   CGDATA
    stz.w   CGDATA

    jsr     Text.Console.Init
    Text.Console.PrintConstString("UnTech Unit Tests:")


    lda.b   #15
    sta.w   INIDISP

    plb
// DB = 0x7e

    plp
    sec
    rts
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:
