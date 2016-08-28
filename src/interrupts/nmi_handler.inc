// nmi_handler.inc
// =================

arch snes.cpu

allocate(frameCounter, shadow, 2)

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


    Dma.VBlank()
    ScrollBuffer.VBlank()


    inc.w   frameCounter

    rep     #$30

    plb
    pld
    ply
    plx
    pla

    rti
}


// REQUIRES: DB access shadow
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

