// audio/entity.inc
// =================
//
// Terrific Audio Driver entity macros and subroutines
//
//
// This file is is an untech port from the Terrific Audio Driver ca65 API.
//
// SPDX-FileCopyrightText: © 2025 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2025 Marcus Rowe <undisbeliever@gmail.com>
//
// This software is provided 'as-is', without any express or implied warranty.  In
// no event will the authors be held liable for any damages arising from the use of
// this software.
//
// Permission is granted to anyone to use this software for any purpose, including
// commercial applications, and to alter it and redistribute it freely, subject to
// the following restrictions:
//
//      1. The origin of this software must not be misrepresented; you must not
//         claim that you wrote the original software. If you use this software in
//         a product, an acknowledgment in the product documentation would be
//         appreciated but is not required.
//
//      2. Altered source versions must be plainly marked as such, and must not be
//         misrepresented as being the original software.
//
//      3. This notice may not be removed or altered from any source distribution.
//


namespace Audio {

// Queue the next sound effect to play, with panning.
//
// NOTE: Only 1 sound effect can be played at a time
// NOTE: Lower sound effect IDs take priority over higher sound effect IDs.
//
// IN: A = sfx id (as determined by the sound effect export order in the project file)
// IN: X = pan (only the lower 8 bits are used.  If `pan > TAD_MAX_PAN`, the sound effect will use center pan)
//
// A8
// I unknown
// DB access lowram
// DP unknown
// KEEP: Y, X
a8()
iu()
code()
function QueuePannedSoundEffect__DpUnknown {
    cmp.w   sfxQueue_sfx
    bcs     EndIf
        sta.w   sfxQueue_sfx

        txa
        sta.w   sfxQueue_pan

EndIf:
    rts
}



// Queue the next sound effect to play with center pan (TAD_MAX_PAN/2).
//
// NOTE: Only 1 sound effect can be played at a time.
// NOTE: Lower sound effect IDs take priority over higher sound effect IDs.
//
// IN: A = sfx id (as determined by the sound effect export order in the project file)
//
// A8
// I unknown
// DB access lowram
// DP unknown
// KEEP: Y, X
a8()
iu()
code()
function QueueSoundEffect__DpUnknown {
    cmp.w   sfxQueue_sfx
    bcs     EndIf
        sta.w   sfxQueue_sfx

        lda.b   #TAD_CENTER_PAN
        sta.w   sfxQueue_pan
EndIf:
    rts
}

}


