// vblank/_common.inc
// ==================
//
// V-Blank and WaitFrame routines
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


// Buffer of the INIDISP register
// (byte, shadow, project editable)
allocate(inidispBuffer, shadow, 1)

// If non-zero then show the CPU usage meter on screen
// (byte flag, shadow)
allocate(showCpuUsageMeter, shadow, 1)

// Frame counter, updated on every `WaitFrame` call.
//
// This frame counter will include lag frames in the count as long as NMI is
// enabled.
//
// (uint32, shadow, project editable)
allocate(frameCounter, shadow, 4)

// Value of the INIDISP register at the end of the `WaitFrame` call.
//
// This variable is used to test if we are in ForceBlank or not.
//
// (byte, shadow)
allocate(inidispAfterWaitFrame, shadow, 1)


if {defined COUNT_FREE_CYCLES} {
    // Number of free cycles in a WaitFrame call
    // (word, shadow)
    allocate(freeCycles, shadow, 2)
}


// Enables the display at full brightness, NMI (VBlank) ISR and autoJoy
//
// Does not immediately turn on the display. The screen will be turned
// on during the next VBlank to prevent screen tearing.
//
// REQUIRES: nothing
au()
iu()
code()
function EnableDisplay_Full {
	php

	sep		#$20
a8()

	lda.b	#INIDISP.brightness.mask
	sta.l	inidispBuffer

	plp
	rts
}



// Force blank the display, disabling IRQ interrupts and disabling HDMA.
//
// IRQ interrupts and HDMA are disabled to prevent VRAM/CGRAM/OAM registers
// from unexpectedly changing in the middle of an upload routine.
//
// NOTE: If the system is not in Force-Blank when this routine is executed then
//       it will call `WaitFrame` to flush any DMA queues to prevent screen
//       tearing.
//
// REQUIRES: nothing
// MODIFIES: Disables Interrupts and HDMA, enables Force Blank
au()
iu()
code()
function ForceBlank {
	php
	phb

	sep		#$20
a8()

    lda.b   #0x80
    pha
    plb
	// DB = 0x80

    // Disables any active IRQ interrupts
	lda.b	#NMITIMEN.vBlank | NMITIMEN.autoJoy
	sta.w	NMITIMEN

	Hdma.Deactivate()


    assert(INIDISP.force == 0x80)
    lda.w   inidispBuffer
    and.w   inidispAfterWaitFrame
    bmi     +
        // not in force blank, set force blank bit and wait one Frame
        ora.b   #INIDISP.force
        sta.w   inidispBuffer
        bra     WaitFrame._AfterPHB
    +

    // INIDISP buffer variables indicate we are in force blank

    // Set Force Blank bit of INIDISP anyways (just in case something went wrong)
    sta.w   INIDISP

    // Set INIDISP anyways (just in case)
	stz.w	HDMAEN

	plb
	plp
	rts
}



// Wait until the start of the next frame, then process the various VBlank
// macros.
//
// If in force blank then this routine will skip the wait.
//
// If `showCpuUsageMeter` is non-zero then the screen brightness is changed to
// show the user the scanline that this routine is called at.
//
// If the define `COUNT_FREE_CYCLES` exists then this routine will count
// the number of free cycles in the frame and store it in the variable
// `freeCycles`.
//
// REQUIRES: nothing
// MODIFIES: May enable Force Blank
au()
iu()
code()
function WaitFrame {

    php
	phb
_AfterPHB:
    phd

    sep     #$30
    rep     #$20
a16()
i8()

    ldx.b   #0x80
    phx
    plb
    // DB = 0x80

    lda.w   #0x4300
    tcd
    // DP = 0x4300


    Dma.FinalizeFrame()


	// Increment frameCounter by the number of NMIs counted
	lda.w	NmiHandler.counter
	and.w	#0xff
	clc
	adc.w	frameCounter
	sta.w	frameCounter
	bcc		+
		inc.w	frameCounter + 2
	+

    if {defined COUNT_FREE_CYCLES} {
        // set to 0 just in case we are in force-blank
        stz.w   freeCycles
    }

    // Don't wait for VBlank when in force-blank
    // if (inidispBuffer & INIDISP.force) and (inidispAfterWaitFrame and INIDISP.force) Goto SkipWaitForNMI
    assert(INIDISP.force == 0x80)
    ldy.w   inidispBuffer
    bpl     +
        ldx.w   inidispAfterWaitFrame
        bmi     SkipWaitForNMI
    +

		// Screen is active

        // Show CPU usage meter (if enabled)
        ldx.w   showCpuUsageMeter
        beq     +
			// Y = inidispBuffer
			tya
            eor.w   #7
            tay
            sty.w   INIDISP
        +

		ldy.b	#INIDISP.force

        // Wait until NmiHandler increments NMI counter

        if {defined COUNT_FREE_CYCLES} {
            // Project wants a freeCycle counter

            // MUST NOT MODIFY Y

	        ldx.w	NmiHandler.counter
            cpx.b   #1
            bne     InALagFrame

                // This frame did not lag, count free cycles

                // Reset NmiHandler counter
                ldx.b   #0
                stx.w   NmiHandler.counter

                lda.w   #0
                CountLoop:
                    inc

                    ldx.w   NmiHandler.counter
                    beq     CountLoop

                sta.w   freeCycles

                bra     SkipWaitForNMI


        InALagFrame:
            // In a lag frame, that means we have no free cycles.
            // Set freeCycles to zero and sleep until the next frame.
            stz.w   freeCycles
        }

        // MUST NOT MODIFY Y

        // Reset NmiHandler counter
        ldx.b   #0
        stx.w   NmiHandler.counter
            Loop:
                wai
                ldx.w   NmiHandler.counter
                beq     Loop

a16()
i8()
SkipWaitForNMI:
    // DB = 0x80
    // DP = 0x4300

    // Y has INIDISP.force bit set

    // Force blank to prevent PPU corruption if VBlank overruns.
    sty.w   INIDISP

	// Disable HDMA
	ldx.b	#0
	stx.w	HDMAEN

	// Processing the VBlank macros in the WaitFrame routine is slightly faster
	// than processing them in the NMI Handler as we do not need to:
	//   * Save the CPU registers to stack
	//   * Setup the DP and DB registers, or
	//   * Test is it is safe to execute the VBlank macros

    Dma.VBlank()
    Hdma.VBlank()

    MetaTiles.Render.DrawMapCell_VBlank()

    Warnings.VBlank()
    ScrollBuffer.VBlank()

    ldx.w   inidispBuffer
    stx.w   INIDISP

    Controller.VBlank()

    Dma.ResetQueues()


    pld
	plb
    plp

    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

