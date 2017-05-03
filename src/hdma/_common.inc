
scope Hdma {

// Initializes the module
//
// REQUIRES: DB access shadow
macro Init() {
    stz.w   Hdma.ChannelSettings.enabled
}


// Deactivate the HDMA buffer
//
// REQUIRES: DB access shadow
macro Deactivate() {
    stz.w   Hdma.ChannelSettings.enabled
}


// Retrieve the next Hdma Table Buffer.
// The buffer returned alternates each function call.
//
// OUTPUT: X = address of Hdma Table Buffer
//
// REQUIRES: 8 bit A, 16 bit X, DB = $7E
a8()
i16()
code()
scope GetBuffer: {
    lda.w   currentBuffer
    bne     +
        dec.w   currentBuffer
        ldx.w   #buffer1
        rts
    +

    stz.w   currentBuffer
    ldx.w   #buffer2
    rts
}



// VBlank code.
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro VBlank() {
    assert16a()
    assert8i()

    scope Hdma {
        ldx.w   Hdma.ChannelSettings.enabled
        beq     Skip
            assert(Hdma.ChannelSettings.dmap + 1 == Hdma.ChannelSettings.register)

            lda.w   Hdma.ChannelSettings.dmap
            sta.b   DMAP7

            lda.w   Hdma.ChannelSettings.tableAddr
            sta.b   A1T7

            ldx.b   #Hdma.buffer1 >> 16
            stx.b   A1B7

            ldx.b   #HDMAEN.dma7
            stx.w   HDMAEN
    Skip:
    }
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:
