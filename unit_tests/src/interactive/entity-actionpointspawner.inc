// SPDX-FileCopyrightText: © 2016 Marcus Rowe <undisbeliever@gmail.com>
// SPDX-License-Identifier: Zlib
//
// Copyright © 2016 Marcus Rowe <undisbeliever@gmail.com>
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

// Tests:
//  * The Entity Loop
//  * MetaSprite ActionPoints
//  * ActionPoints.PlayerProjectiles
//  * ActionPoints.EnemyProjectiles
//  * ActionPoints.SpawnEntityFunction
//  * ActionPoints.ManualEnemyProjectile
//  * Entity.BaseEntityRomStruct.initialProjectileId
//  * Entity.SpawnManualEnemyProjectiles
//  * Entity.DeleteIfOutsideAndGotoNextEntity (in the EightDirectionRedProjectile)
//  * Entity.CountEntitiesInList
//  * Entity.MoveEntity() (Entity.MovementState.NO_GRAVITY_FLAG set)


namespace Entities {
namespace Projectiles {

namespace EightDirectionProjectile {
    buildFunctionTable(BaseEntityFunctionTable, EightDirectionProjectile)

    // Slow speed to exhaust entity pool.
    constant VELOCITY = 0x8000
    constant DIAGONAL_VELOCITY = 0x5a82 // sqrt(0x8000 ** 2 / 2

    namespace EntityStruct {
        childstruct(BaseEntity)
        endstruct()
    }

    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        // Confirm entity momentum is a 24 bit value
        assert(BaseEntity.yMomentum.sx - BaseEntity.xMomentum.sx == 3)

        sep     #$20
    a8()

        // Ensure initialProjectileId is 0xff (unused)
        lda.b   EntityStruct.projectileId
        cmp.b   #0xff
        beq     +
            break(ASSERT_FAILURE)
        +

        // Determine if projectile is diagonal or orthogonal
        lda.w   MetaSprite.ActionPoint.positionPair.xPos
        cmp.b   #INT_MS8_OFFSET
        beq     Orthogonal_Y

        lda.w   MetaSprite.ActionPoint.positionPair.yPos
        cmp.b   #INT_MS8_OFFSET
        beq     Orthogonal_X

            // Projectile is Diagonal
            ldy.w   #DIAGONAL_VELOCITY
            sty.b   EntityStruct.xMomentum.sx
            sty.b   EntityStruct.yMomentum.sx
            ldy.w   #DIAGONAL_VELOCITY >> 8
            sty.b   EntityStruct.xMomentum.sx + 1
            sty.b   EntityStruct.yMomentum.sx + 1

            bra     EndIf

        Orthogonal_X:
            // Projectile is Orthogonal in the X axis
            ldy.w   #VELOCITY
            sty.b   EntityStruct.xMomentum.sx
            ldy.w   #VELOCITY >> 8
            sty.b   EntityStruct.xMomentum.sx + 1

            bra     EndIf

        Orthogonal_Y:
            // Projectile is Orthogonal in the Y axis
            ldy.w   #VELOCITY
            sty.b   EntityStruct.yMomentum.sx
            ldy.w   #VELOCITY >> 8
            sty.b   EntityStruct.yMomentum.sx + 1

            bra     EndIf
    EndIf:


        // Set movement state
        lda.b   #Entity.MovementState.NO_GRAVITY_FLAG
        sta.b   EntityStruct.movementState

        lda.w   MetaSprite.ActionPoint.positionPair.xPos
        cmp.b   #INT_MS8_OFFSET
        bcc     +
            lda.b   #Entity.MovementState.MOVING_RIGHT_FLAG
            tsb.b   EntityStruct.movementState
        +

        lda.w   MetaSprite.ActionPoint.positionPair.yPos
        cmp.b   #INT_MS8_OFFSET
        bcc     +
            lda.b   #Entity.MovementState.MOVING_DOWN_FLAG
            tsb.b   EntityStruct.movementState
        +


        rep     #$30
    a16()

        SetFrame(frame)

        // always activate
        sec
        rts
    }

    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
    function Null {
        rts
    }

    a16()
    i16()
    code()
    Process: {
        MoveEntity()

        jmp     Entity.DeleteIfOutsideOrDrawAndGotoNextEntity
    }
}
}

// Reuse EightDirectionProjectile code in EightDirectionEntity
constant EightDirectionEntity.FunctionTable = Projectiles.EightDirectionProjectile.FunctionTable


namespace Players {
namespace ActionPointSpawner {
    buildFunctionTable(BasePlayerFunctionTable, ActionPointSpawner)

    constant DrawPlayer = MetaSprite.Render.RenderEntity

    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
        SetAnimation(animation)

        // Ensure initialProjectileId is EightDirectionRedProjectile
        assert({GetProjectileId(EightDirectionRedProjectile)} == Project.ProjectileIds.EightDirectionRedProjectile)
        assert({GetProjectileId(EightDirectionRedProjectile)} != 0)
        lda.b   BaseEntity.projectileId
        and.w   #0xff
        cmp.w   #{GetProjectileId(EightDirectionRedProjectile)}
        beq     +
            break(ASSERT_FAILURE)
        +

        rts
    }

    a16()
    i16()
    code()
    RoomAboutToChange:
    HitboxCollision:
    HurtboxCollision:
    ShieldCollision:
    function Null {
        rts
    }

    // Spawn the projectile using alternating action points and
    // spawn routines.
    // If no entity was spawned then try again on the next frame.
    a16()
    i16()
    code()
    function Process {
        lda.w   Controller.Joy1.pressed
        and.w   #JOY.shoulder
        beq     +
            // When L or R is pressed spawn 4 green projectiles

            sep     #$20
        a8()
            SetProjectileId(EightDirectionGreenProjectile)

            rep     #$30
        a16()

            SpawnManualEnemyProjectiles()

            sep     #$10
        i8()
            SetProjectileId(EightDirectionRedProjectile)

            rep     #$30
        i16()
        +

        jmp     MetaSprite.Animation.Process
    }
}
}
}


namespace InteractiveTests {
namespace ActionPointSpawnerTest {

// Prints the list statistics to screen
macro PrintListSizes() {
    assert16a()
    assert16i()

    Text.Console.SetCursor(0, 20)
    Text.Console.PrintConstString("\nPlayer Projectiles: ")

    Entity.CountEntitiesInList(PlayerProjectiles)
    jsr     Text.Console.PrintU16A

    Text.Console.PrintConstString("\nEnemy Projectiles:  ")

    Entity.CountEntitiesInList(EnemyProjectiles)
    jsr     Text.Console.PrintU16A

    Text.Console.PrintConstString("\nParticle Entities:  ")

    Entity.CountEntitiesInList(Particles)
    jsr     Text.Console.PrintU16A
}


// The Test
a16()
i16()
code()
Test.add("ActionPointSpawner Test")
function Test {
    lda.w   #Project.RoomList.EmptyRoom
    ldx.w   #Project.PlayerIds.ActionPointSpawner
    ldy.w   #0
    jsr     GameLoop.Init_LoadRoom


    jsr     Resources.TransferToPpu
    jsl     MetaTiles.Render.DrawFullScreen_ForceBlank__far

    jsr     Test.PrintCurrentTestName

    Text.Console.SetCursor(0, 7)
    Text.Console.PrintConstString("Press L/R to spawn\n  Manual Projectiles")

    jsr     EnableDisplay_Full


    // Process the Game Loop
    // ---------------------

    Loop:
        jsr     GameLoop.ProcessFrame

        PrintListSizes()

        jsr     WaitFrame_PrintFreeCycles

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        beq     Loop


    jmp     GameLoop.Cleanup

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

