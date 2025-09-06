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


namespace Entities {
namespace CrewRescued {


constant SPRITE_XPOS = 128 - INT_MS8_OFFSET
constant SPRITE_YPOS = 80 - INT_MS8_OFFSET

constant FRAME_0_DISPLAY_COUNTDOWN = 0
constant FRAME_1_DISPLAY_COUNTDOWN = 45

constant FRAMES_UNTIL_NEXT_LEVEL = 5 * 60


buildFunctionTable(BaseEntityFunctionTable, CrewRescued)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(frame, 2)

        field(displayCountdown, 2)
    endstruct()
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//         A = top/bottom half
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    and.w   #1
    sta.b   EntityStruct.frame
    SetFrame_A()

    sep     #$20
a8()
    lda.b   #1
    sta.w   crewRescuedFlag

    ldx.w   #FRAME_1_DISPLAY_COUNTDOWN

    lda.b   EntityStruct.frame
    bne     +
        Entities.Players.Defeated.SwitchToVictoriousPlayer()

        lda.b   #Song.VictoryTheme
        jsr     Audio.LoadSong

        rep     #$30
    a16()

        lda.w   #Project.EntityIds.crew_rescued
        ldy.w   #1
        jsr     Entity.Spawn

        ldx.w   #FRAME_0_DISPLAY_COUNTDOWN
    +

au()
    stx.b   EntityStruct.displayCountdown

    rep     #$30
a16()

    ldx.w   #SPRITE_XPOS
    stx.b   EntityStruct.xPos.px

    ldx.w   #SPRITE_YPOS
    stx.b   EntityStruct.yPos.px

    sec
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Destructor {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Deactivated {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HitboxCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with shield
a16()
i16()
code()
function HurtboxCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//        Y = other entity
//        Carry set if collision with hitbox
a16()
i16()
code()
function ShieldCollision {
    rts
}


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process {
    dec.b   EntityStruct.displayCountdown
    bpl     +
        lda.b   EntityStruct.xPos.px
        sta.w   MetaSprite.Render.xPos

        lda.b   EntityStruct.yPos.px
        sta.w   MetaSprite.Render.yPos

        ldx.b   EntityStruct.MetaSprite.currentFrame
        jsr     MetaSprite.Render.RenderEntityFrameAndPos
    +

    jmp     Entity.GotoNextEntity
}

}
}
