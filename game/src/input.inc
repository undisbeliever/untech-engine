// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
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


constant JOY_H_RUN_BUTTON  = JOYH.y


namespace InputBuffer {

constant JOY_L_ATTACK_BUTTON      = JOYL.a
constant JOY_L_PROJECTILE_BUTTON  = JOYL.x

constant JOY_H_JUMP_BUTTON = JOYH.b

// ::TODO adjust::
constant ATTACK_BUFFER_FRAMES = 3
constant PROJECTILE_BUFFER_FRAMES = 5
constant JUMP_BUFFER_FRAMES = 5


macro Process() {
    assert8a()
    assert8i()


    lsr.b   InputBuffer.attack
    lsr.b   InputBuffer.projectile
    lsr.b   InputBuffer.jump


    assert(InputBuffer.JOY_L_ATTACK_BUTTON == 0x80)
    lda.w   Controller.Joy1.pressed
    bpl     +
        assert(InputBuffer.ATTACK_BUFFER_FRAMES < 9)
        ldx.b   #1 << (InputBuffer.ATTACK_BUFFER_FRAMES - 1)
        stx.b   InputBuffer.attack
    +

    bit.b   #InputBuffer.JOY_L_PROJECTILE_BUTTON
    beq     +
        assert(InputBuffer.PROJECTILE_BUFFER_FRAMES < 9)
        ldx.b   #1 << (InputBuffer.PROJECTILE_BUFFER_FRAMES - 1)
        stx.b   InputBuffer.projectile
    +


    lda.w   Controller.Joy1.pressed + 1
    bit.b   #InputBuffer.JOY_H_JUMP_BUTTON
    beq     +
        assert(InputBuffer.JUMP_BUFFER_FRAMES < 9)
        ldx.b   #1 << (InputBuffer.JUMP_BUFFER_FRAMES - 1)
        stx.b   InputBuffer.jump
    +
}


}
