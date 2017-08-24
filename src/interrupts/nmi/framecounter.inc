// interrupts/nmi/framecounter.inc
// ===============================
//
// Frame counter, incremented every VBlank.
//
//
// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


allocate(frameCounter, shadow, 2)

namespace NmiHandler {
// VBlank code, increase frameCounter
//
// REQUIRES: 16 bit A, 8 bit Index, DB = $80, DP = $4300
macro FrameCounter.VBlank() {
    assert16a()

    inc.w   frameCounter
}
}

// REQUIRES: DB access shadow, VBlank Enabled
au()
iu()
code()
function WaitFrame {
    lda.w   frameCounter

    -
        wai
        cmp.w   frameCounter
        beq     -

    rts
}

// vim: ft=bass-65816 ts=4 sw=4 et:

