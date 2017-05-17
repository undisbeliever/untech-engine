// interrupts/nmi/scrollbuffer.inc
// ===============================
//
// VBlank buffer for the scroll registers.


scope ScrollBuffer {
scope bg1 {
    allocate(hOffset, shadow, 2)
    allocate(vOffset, shadow, 2)
}
scope bg2 {
    allocate(hOffset, shadow, 2)
    allocate(vOffset, shadow, 2)
}


// VBlank code, updates scroll registers
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    ldx.w   ScrollBuffer.bg1.hOffset
    stx.w   BG1HOFS
    ldx.w   ScrollBuffer.bg1.hOffset + 1
    stx.w   BG1HOFS

    ldx.w   ScrollBuffer.bg1.vOffset
    stx.w   BG1VOFS
    ldx.w   ScrollBuffer.bg1.vOffset + 1
    stx.w   BG1VOFS

    ldx.w   ScrollBuffer.bg2.hOffset
    stx.w   BG2HOFS
    ldx.w   ScrollBuffer.bg2.hOffset + 1
    stx.w   BG2HOFS

    ldx.w   ScrollBuffer.bg2.vOffset
    stx.w   BG2VOFS
    ldx.w   ScrollBuffer.bg2.vOffset + 1
    stx.w   BG2VOFS
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

