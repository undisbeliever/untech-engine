// interrupts/nmi/controller.inc
// =============================
//
// Controller button State
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace Controller {

namespace Joy1 {
	// The new buttons that were pressed on the current `UpdateControllerState`
	allocate(pressed, shadow, 2)

	// The currently pressed buttons on the joypad
	allocate(current, shadow, 2)

	allocate(_invertedPrevious, shadow, 2)
	allocate(_buffer, shadow, 2)
}

if {defined CONTROLLER_USE_JOY2} {
	namespace Joy2 {
		allocate(pressed, shadow, 2)
		allocate(current, shadow, 2)

		allocate(_invertedPrevious, shadow, 2)
		allocate(_buffer, shadow, 2)
	}
}

// Update joypad buffer
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80
macro VBlank() {
	assert16a()
    assert(HVBJOY.autoJoy == 1)

    -
        ldx.w	HVBJOY			// MUST do 8 bit read on HVBJOY
        txa
        lsr
        bcs		-

    lda.w   JOY1
    tsb.w   Controller.Joy1._buffer

    if {defined CONTROLLER_USE_JOY2} {
        lda.w   JOY2
        tsb.w   Controller.Joy2._buffer
	}
}

// Updates the controller state variables.
//
// This should be called in the GameLoop NOT VBlank so inputs are not dropped.
//
// REQUIRES: 16 bit A, 16 bit Index, DB access shadow
macro UpdateControllerState() {
    assert16a()
    assert16i()

	Controller._update(Joy1)

	if {defined CONTROLLER_USE_JOY2} {
		Controller._update(Joy2)
	}
}

macro _update(joy) {
    assert16a()
    assert16i()

	// ensure device connected to JOY1 is a controller
	assert(JOY.type.controller == 0)

	lda.w	Controller.{joy}._buffer
	bit.w	#JOY.type.mask
	beq		+
		lda.w	#0
	+

	tay
	sta.w   Controller.{joy}.current
	and.w   Controller.{joy}._invertedPrevious
	sta.w   Controller.{joy}.pressed

	tya
	eor.w   #0xffff
	sta.w   Controller.{joy}._invertedPrevious

	stz.w	Controller.{joy}._buffer
}

}

// vim: ft=bass-65816 ts=4 sw=4 et:

