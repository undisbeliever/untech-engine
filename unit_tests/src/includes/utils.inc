// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT



// Wait until the start of the next frame, printing the number
// of loops waited on the top-right of the screen.
//
// REQUIRES: DB access shadow
//
// RETURN: Y = number of loops waited
code()
function WaitFrame_PrintFreeCycles {
    Text.Console.SetCursor(Text.Console.TEXT_WIDTH - 5, 0)

    // Using hex as it is constant time

    jsr     WaitFrame

    ldy.w   freeCycles
    jmp     Text.Console.PrintHex16Y
}

// vim: ft=bass-65816 ts=4 sw=4 et:

