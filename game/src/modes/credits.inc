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

namespace CreditsScreen {


a16()
i16()
// DB = 0x7e
code()
function Run {
    sep     #$20
a8()
    lda.b   #Song.EndingTheme
    jsr     Audio.LoadSong

    jsr     FadeOut


    rep     #$30
a16()

    lda.w   #Project.Scenes.credits
    jsr     Resources.LoadScene
    jsr     Resources.TransferToPpu

    jsr     FadeIn

    Loop:
        jsl     WaitFrame__far

        lda.w   Controller.Joy1.pressed
        bit.w   #JOY.start | JOY.b | JOY.a
        beq     Loop


    lda.w   #Mode.TITLE_SCREEN
    rts
}

}

