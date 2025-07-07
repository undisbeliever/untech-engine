// vblank/_common.inc
// ==================
//
// V-Blank and WaitFrame routines
//
//
// SPDX-FileCopyrightText: © 2019 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2019 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//    1. The origin of this software must not be misrepresented; you must not
//       claim that you wrote the original software. If you use this software in
//       a product, an acknowledgment in the product documentation would be
//       appreciated but is not required.
//
//    2. Altered source versions must be plainly marked as such, and must not be
//       misrepresented as being the original software.
//
//    3. This notice may not be removed or altered from any source distribution.


// Buffer of the INIDISP register
// (byte, lowram, project editable)
allocate(inidispBuffer, lowram, 1)

// Frame counter, updated on every `WaitFrame` call.
//
// This frame counter will include lag frames in the count as long as NMI is
// enabled.
//
// (uint32, lowram, project editable)
allocate(frameCounter, lowram, 4)

// Value of the INIDISP register at the end of the `WaitFrame` call.
//
// This variable is used to test if we are in ForceBlank or not.
//
// (byte, lowram)
allocate(inidispAfterWaitFrame, lowram, 1)


if {defined COUNT_FREE_CYCLES} {
    // Number of free cycles in a WaitFrame call
    // (word, lowram)
    allocate(freeCycles, lowram, 2)
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
farCode()
function ForceBlank__far {
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
        bra     WaitFrame__far._AfterPHB
    +

    // INIDISP buffer variables indicate we are in force blank

    // Set Force Blank bit of INIDISP anyways (just in case something went wrong)
    sta.w   INIDISP

    // Set INIDISP anyways (just in case)
	stz.w	HDMAEN

	plb
	plp
	rtl
}



// Wait until the start of the next frame, then process the various VBlank
// macros.
//
// If in force blank then this routine will skip the wait.
//
// If the define `COUNT_FREE_CYCLES` exists then this routine will count
// the number of free cycles in the frame and store it in the variable
// `freeCycles`.
//
// REQUIRES: nothing
// MODIFIES: May enable Force Blank
au()
iu()
farCode()
function WaitFrame__far {

    php
	phb
_AfterPHB:
    phd

    sep     #$20
    rep     #$10
a8()
i16()

    lda.b   #0x80
    pha
    plb
// DB = 0x80

    jsl     Audio.Process__far


    sep     #$30
    rep     #$20
a16()
i8()

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

        // Calculate inidispBuffer | INIDISP.force
        // Y = inidispBuffer
        tya
        ora.w   #INIDISP.force
        tay


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

    // Force blank to prevent PPU corruption if VBlank overruns.
	//
	// 1CHIP consoles may display a glitched a darkened band on the top of the
	// screen if I set INIDISP to 0x80 here.  To prevent this glitch the
	// brightness bits of INIDISP should be unchanged when enabling the display
	// at the end of VBlank.
    //
    // Example of the glitch (see Street Fighter 2 pics):
    //   https://shmups.system11.org/viewtopic.php?p=1287176#p1287176
	//
	// Explanation of the glicth:
	//   https://shmups.system11.org/viewtopic.php?p=1309776#p1309776

    // Y = inidispBuffer | INIDISP.force
    sty.w   INIDISP


	// Disable HDMA
	ldx.b	#0
	stx.w	HDMAEN

	// Processing the VBlank macros in the WaitFrame routine is slightly faster
	// than processing them in the NMI Handler as we do not need to:
	//   * Save the CPU registers to stack
	//   * Setup the DP and DB registers, or
	//   * Test is it is safe to execute the VBlank macros

    Dma.VBlank_dp4300()
    Hdma.VBlank_dp4300()


    // MUST NOT USE DMA PAST THIS POINT
    // --------------------------------

    MetaTiles.Render.DrawMapCell_VBlank_dp4300()

    lda.w   #0x2100
    tcd
// DP = $2100

    Resources.VBlank_dp2100()
    Warnings.VBlank_dp2100()


    ldx.w   inidispBuffer
    stx.b   INIDISP

    Dma.ResetQueues()


    Controller.VBlank_dp2100_set16I()

assert16a()
assert16i()

    pld
    plb
    plp

    rtl
}

// vim: ft=bass-65816 ts=4 sw=4 et:

