// vblank/controller.inc
// =====================
//
// Controller button State
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


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

