// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


namespace InteractiveTests {
namespace EntityValidation {

// Tests:
//  * Entity.ValidateEntityLists

constant cursorIndex = Test.dpTmp + 0
constant menuLineIndex = Test.dpTmp + 2


constant N_MENU_ITEMS = 9


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _Init() {
    assert16a()
    assert16i()

    jsr     EntityTest._SetupCounterTest
    bcc     Fail
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
macro _PrintMenu() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(0, 4)

    ldx.w   #0

    Loop:
        stx.b   menuLineIndex

        cpx.b   cursorIndex
        bne     +
            ldx.w   #CursorString
            bra     ++
        +
            ldx.w   #NoCursorString
        +
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        ldx.b   menuLineIndex
        lda.l   MenuTexts,x
        tax
        lda.w   #CursorString >> 16
        jsr     Text.Console.PrintString

        jsr     Text.Console.NewLine

        ldx.b   menuLineIndex
        inx
        inx
        cpx.w   #N_MENU_ITEMS * 2
        bcc     Loop
}


namespace Test {
Fail:
    clc
    rts
}

a16()
i16()
code()
Test.add("Entity Validation")
function Test {

    _Init()


    ldx.w   #0

    PrintMenuAndLoop:
        stx.b   cursorIndex

        _PrintMenu()

        Loop:
            jsr     WaitFrame

            Controller.UpdateControllerState()

            lda.w   Controller.Joy1.pressed
            beq     Loop


            ldx.b   cursorIndex

            bit.w   #JOY.up
            beq     +
                dex
                dex
                bpl     +
                    ldx.w   #(N_MENU_ITEMS - 1) * 2
            +
            // A = Joy1
            bit.w   #JOY.down
            beq     +
                inx
                inx
                cpx.w   #N_MENU_ITEMS * 2
                bcc     +
                    ldx.w   #0
            +

            // A = Joy1
            // X = new cursorIndex
            bit.w   #JOY.buttons
            bne     EndLoop

            jmp     PrintMenuAndLoop

EndLoop:
    // button pressed
    // X = new cursorIndex
    jsr     (MenuRoutines,x)

    jsr     Entity.ValidateEntityLists

    sec
    rts
}


rodata(rom0)
CursorString:
    db  " >", 0

NoCursorString:
    db  "  ", 0

rodata(rom0)
MenuTexts:
    evaluate n = 0
    while {n} < N_MENU_ITEMS {
        dw  MenuText{n}
        evaluate n = {n} + 1
    }

code()
MenuRoutines:
    evaluate n = 0
    while {n} < N_MENU_ITEMS {
        dw  MenuRoutine{n}
        evaluate n = {n} + 1
    }


rodata(rom0)
MenuText0:
    db  "None", 0
a16()
i16()
code()
function MenuRoutine0 {
    rts
}


rodata(rom0)
MenuText1:
    db  "InfiniteLoop (1st enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine1 {
    ldy.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2

    jmp     _CreateInfiniteLoop_Y
}


rodata(rom0)
MenuText2:
    db  "InfiniteLoop (2nd enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine2 {
    ldx.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2
    ldy.b   BaseEntity.next,x

    jmp     _CreateInfiniteLoop_Y
}


rodata(rom0)
MenuText3:
    db  "InfiniteLoop (free list)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine3 {
    ldy.w   Entity.lists.FIRST + Entity.lists._FREE_INDEX

    jmp     _CreateInfiniteLoop_Y
}


rodata(rom0)
MenuText4:
    db  "Drop Entity (enemy)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine4 {
    // enemy->first = enemy->first->next

    ldx.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2

    lda.b   BaseEntity.next,x
    sta.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2

    rts
}


rodata(rom0)
MenuText5:
    db  "Drop Entity (deactivated)", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine5 {
    // deactivated->first = deactivated->first->next

    ldx.w   Entity.lists.FIRST + Entity.lists._DEACTIVATED_INDEX

    lda.b   BaseEntity.next,x
    sta.w   Entity.lists.FIRST + Entity.lists._DEACTIVATED_INDEX

    rts
}


rodata(rom0)
MenuText6:
    db  "Deactivated in boss list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine6 {
    // verify boss list empty
    -
        sta.w   Entity.lists.FIRST + Entity.ListId.BOSS * 2
        beq     -

    // boss->first = deactivated->first
    // deactivated->first = null

    lda.w   Entity.lists.FIRST + Entity.lists._DEACTIVATED_INDEX
    sta.w   Entity.lists.FIRST + Entity.ListId.BOSS * 2

    stz.w   Entity.lists.FIRST + Entity.lists._DEACTIVATED_INDEX

    rts
}


rodata(rom0)
MenuText7:
    db  "Activated in free list", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine7 {
    // free->last->next = enemy->first
    // enemy->first = null

    ldy.w   Entity.lists.FIRST + Entity.lists._FREE_INDEX
    Loop:
        tyx
        ldy.b   BaseEntity.next,x
        bne     Loop

    // X = last item in free list
    lda.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2
    sta.b   BaseEntity.next,x

    stz.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2

    rts
}


rodata(rom0)
MenuText8:
    db  "Invalidate an enemy listId", 0

// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
a16()
i16()
code()
function MenuRoutine8 {
    // enemy->next->listId = Entity.ListId.PLAYER

    ldy.w   Entity.lists.FIRST + Entity.ListId.ENEMY * 2
    ldx.b   BaseEntity.next,y

    // X = second entity in enemy list

    sep     #$20
a8()
    lda.b   #Entity.ListId.PLAYER
    sta.b   BaseEntity.listId,x

    rep     #$30
a16()

    rts
}



// REQUITES: 16 bit A, 16 bit Index, DB = 0x7e, DP = 0
// INPUT: Y = entity in list
a16()
i16()
code()
function _CreateInfiniteLoop_Y {
    tya
    beq     Return

        Loop:
            tyx
            ldy.b   BaseEntity.next,x
            bne     Loop

        // A = parameter entity
        // X = last entity in list
        sta.b   BaseEntity.next,x

Return:
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

