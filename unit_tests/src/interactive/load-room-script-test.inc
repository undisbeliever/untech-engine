// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//   * Load_Room scripting bytecode instruction


namespace InteractiveTests {
namespace LoadRoomScriptTest {


constant GRAVITY = 10000

constant TEST_TIME = 80


a16()
i16()
code()
Test.add("Load_Room Script Test")
function Test {

constant _timer  = Test.dpTmp + 0


    lda.w   #Project.RoomList.LoadRoomScriptTest_1
    ldx.w   #Project.PlayerIds.InteractiveTilesTest_Player
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    lda.w   #GRAVITY
    sta.w   Room.gravity_sx


    jsr     Resources.TransferToPpu
    jsr     MetaTiles.Render.DrawFullScreen_ForceBlank

    jsr     EnableDisplay_Full


    lda.w   #TEST_TIME
    sta.b   _timer

    Loop:
        jsr     WaitFrame_PrintFreeCycles

        jsr     GameLoop.ProcessFrame


        jsr     Test.PrintCurrentTestName

        Text.Console.SetCursor(0, 4)
        Text.Console.PrintConstString("RoomId: ")

        lda.w   GameState.roomId
        and.w   #0xff
        jsr     Text.Console.PrintU16A


        dec.b   _timer
        bne     Loop


    // Confirm we are in the third room in the Load_Room chain
    lda.w   GameState.roomId
    and.w   #0xff
    cmp.w   #Project.RoomList.LoadRoomScriptTest_3
    bne     Fail


    jsr     Entity.ValidateEntityLists

    jmp     Test.ResetPpuState


Fail:
    clc
    rts
}


}
}

