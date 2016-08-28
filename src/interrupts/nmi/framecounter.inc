
allocate(frameCounter, shadow, 2)

scope NmiHandler {
// VBlank code, increase frameCounter
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro FrameCounter.VBlank() {
    assert16a()

    inc.w   frameCounter
}
}

// REQUIRES: DB access shadow, VBlank Enabled
code()
scope WaitFrame: {
    lda.w   frameCounter

    -
        wai
        cmp.w   frameCounter
        beq     -

    rts
}

// vim: ft=asm ts=4 sw=4 et:

