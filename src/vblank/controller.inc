// vblank/controller.inc
// =====================
//
// Controller button State
//
//
// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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


namespace Controller {

namespace Joy1 {
	// The state of the joypad for the current frame
    // (word, lowram)
	allocate(current, lowram, 2)

	// The new buttons that were pressed for the current frame
    // (word, lowram)
	allocate(pressed, lowram, 2)
}

if {defined CONTROLLER_USE_JOY2} {
	namespace Joy2 {
		allocate(current, lowram, 2)
		allocate(pressed, lowram, 2)
	}
}


// Update a single joypad variables
//
// REQUIRES: 16 bit A, 16 bit Index, DB = $80
macro _UpdateState(joyRegister, joyStruct) {
    assert16a()
    assert16i()

	// Ignore any device that is not a controller
	assert(JOY.type.controller == 0)
    lda.w   {joyRegister}
	bit.w	#JOY.type.mask
	beq		+
		lda.w	#0
	+

    // invertedPrevious = joyStruct.current ^ 0xffff
    // joyStruct.current = joyRegister
    // joyStruct.pressed = invertedPrevious & joyStruct.current

    tay
    lda.w   {joyStruct}.current
    eor.w   #0xffff
    sty.w   {joyStruct}.current
    and.w   {joyStruct}.current
    sta.w   {joyStruct}.pressed
}


// Update joypad state
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, dp = 0x2100
// MODIFIES: 16 bit A, 16 bit Index
macro VBlank_dp2100_set16I() {
	assert16a()
    assert8i()

    // Wait until autoJoy has finished
    sep     #$20
a8()
    -
        lda.w	HVBJOY			// MUST do 8 bit read on HVBJOY
        and.b   #HVBJOY.autoJoy
        bne		-

    rep     #$30
a16()
i16()

    Controller._UpdateState(JOY1, Controller.Joy1)

    if {defined CONTROLLER_USE_JOY2} {
        Controller._UpdateState(JOY2, Controller.Joy2)
	}
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

