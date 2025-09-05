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
namespace BossNet {


// 8.8 Fixed point
constant STARTING_Y_MOMENTUM = 0x00b0


// ::DBEUG code::

buildFunctionTable(BaseEntityFunctionTable, BossNet)


namespace EntityStruct {
    childstruct(BaseEntity)
        field(bossInstanceId, 1)

        // flag
        field(unfolded, 2)

        // flag
        field(crewRescuedSpawned, 2)
    endstruct()
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function NullFunction {
    rts
}

constant Destructor = NullFunction
constant Deactivated = NullFunction
constant HitboxCollision = NullFunction
constant HurtboxCollision = NullFunction
constant ShieldCollision = NullFunction


// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
//         A = boss entityId
// OUTPUT: C set = always activate entity
a16()
i16()
code()
function Init {
    sep     #$20
a8()
    sta.b   EntityStruct.bossInstanceId

    lda.b   #Gravity.BOSS_NET
    sta.b   EntityStruct.gravity

    QueueSoundEffect(throwNet)


    rep     #$30
a16()

    stz.b   EntityStruct.unfolded
    stz.b   EntityStruct.crewRescuedSpawned

    lda.w   #STARTING_Y_MOMENTUM
    sta.b   EntityStruct.yMomentum.px - 1

    lda.w   #{GetFrameId(small)}
    jmp     MetaSprite.SetFrame
}



// REQUIRES: 16 bit A, 16 bit Index, DB = 0x7e
// INPUT: DP = entity
a16()
i16()
code()
function Process {
    ldy.b   EntityStruct.yPos.px
    cpy.w   #0x1030
    bcc     Move
        cpy.w   #0x1088
        bcc     +
            lda.b   EntityStruct.crewRescuedSpawned
            bne     End
                sep     #$20
            a8()
                inc.b   EntityStruct.crewRescuedSpawned

                QueueSoundEffect(bossHurt)

                rep     #$30
            a16()

                // ::TODO Spawn CREW RESCUED sprite::

                bra     End
        +

        lda.b   EntityStruct.unfolded
        bne     +
            inc.b   EntityStruct.unfolded
            SetAnimation(unfold)
        +

Move:
    lda.b   EntityStruct.bossInstanceId
    Entity.GetEntityWithIdOrBranchIfEmpty_Y(EndXMove)
        ldx.w   BaseEntity.xPos.px,y
        cpx.b   EntityStruct.xPos.px
        beq     EndXMove
        bcc     +
            inc.b   BaseEntity.xPos.px
            bra     EndXMove
        +
            dec.b   BaseEntity.xPos.px
    EndXMove:

    MoveEntity()

End:
    jmp     Entity.DrawAndGotoNextEntity
}

}
}

