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

namespace LevelNameScreen {

// Current level
allocate(levelNumber, wram7e, 1)

allocate(restartLevelOnZero, wram7e, 1)


// DB = 0x7e
macro ResetLevelNumber() {
    assert16a()

    assert(LevelNameScreen.restartLevelOnZero == LevelNameScreen.levelNumber + 1)
    stz.w   LevelNameScreen.levelNumber
}


// DB = 0x7e
macro RestartLevel() {
    assert8a()
    stz.w   LevelNameScreen.restartLevelOnZero
}



a16()
i16()
// DB = 0x7e
code()
function Run {
    // Reset player health
    lda.w   #Healthbar.MAX_HEALTH
    sta.w   Project.GameState.Words.playerHealth


    sep     #$20
a8()
    lda.b   #Song.BLANK
    jsr     Audio.LoadSong


    jsr     FadeOut


    rep     #$30
a16()

    lda.w   #Project.Scenes.text
    jsr     Resources.LoadScene
    jsr     Resources.TransferToPpu


    sep     #$20
a8()

    Text.Console.SetCursor(0, 2)


    // Clear Ah
    tdc

    lda.w   levelNumber
    cmp.b   #N_LEVELS
    bcc     +
        lda.b   #0
        stz.w   levelNumber
    +

    ldx.w   #LevelNameData

    cmp.b   #0
    beq     Found
        tay

        ScanLoop:
            inx
            lda.l   LevelNameData & 0xff0000,x
            bne     +
                inx

                dey
                beq     Found
            +
            bra     ScanLoop
    Found:


    lda.w   restartLevelOnZero
    bne     +
        inc.w   restartLevelOnZero

        lda.l   LevelNameData & 0xff0000,x
        sta.w   GameState.roomId
        stz.w   GameState.entranceId
    +

    inx
    lda.b   #LevelNameData >> 16
    jsr     Text.Console.PrintString

    // ::TODO use code for level number::


    jsr     FadeIn


    rep     #$30
a16()
    Loop:
        jsl     WaitFrame__far

        lda.w   Controller.Joy1.pressed
        bit.w   #JOY.start | JOY.b | JOY.a
        beq     Loop


    lda.w   #Mode.GAME
    rts
}


// Format: string name, room index
rodata()
LevelNameData:
    db  Project.RoomList.playtest_1_entrance, "1-1\nIntro", 0
    db  Project.RoomList.a2a_start, "1-2\nGravity Labs", 0
    db  0

constant N_LEVELS = 2
}

