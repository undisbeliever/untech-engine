
scope Controller {

scope Joy1 {
	// The new buttons that were pressed on the current `UpdateControllerState`
	allocate(pressed, shadow, 2)

	// The currently pressed buttons on the joypad
	allocate(current, shadow, 2)

	allocate(_invertedPrevious, shadow, 2)
	allocate(_buffer, shadow, 2)
}

if {defined CONTROLLER_USE_JOY2} {
	scope Joy2 {
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

	scope Controller: {
		-
			ldx.w	HVJOY			// MUST do 8 bit read on HVJOY
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

// vim: ft=asm ts=4 sw=4 et:

