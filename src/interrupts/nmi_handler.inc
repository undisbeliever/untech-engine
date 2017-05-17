// nmi_handler.inc
// =================

arch snes.cpu

au()
iu()
code()
scope NmiHandler: {
    // Store registers
    rep     #$30
a16()
i16()
    pha
    phx
    phy
    phd
    phb

    phk
    plb

    lda.w   #$4300
    tcd

    sep     #$10
i8()

    // Reset NMI Flag.
    ldy.w   RDNMI

    // Disable HDMA
    ldx.b   #0
    stx.w   HDMAEN


    Dma.VBlank()
    Hdma.VBlank()

    ScrollBuffer.VBlank()
    FrameCounter.VBlank()
    Controller.VBlank()


    rep     #$30

    plb
    pld
    ply
    plx
    pla

    rti
}


// Enables NMI (VBlank) Interrupts and autoJoy
//
// REQUIRES: DB access shadow
au()
iu()
code()
scope EnableVBlank: {
    php

    sep     #$20
a8()
    lda.b   #NMITIMEN.vBlank | NMITIMEN.autoJoy
    sta.l   NMITIMEN

    rep     #$30
a16()
    Dma.ResetTransfersLeft()

    plp
    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

