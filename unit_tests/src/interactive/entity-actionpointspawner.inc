// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

// Tests:
//  * EntityLoop
//  * MetaSprite ActionPoints
//  * ActionPoints.PlayerProjectiles
//  * ActionPoints.EnemyProjectiles
//  * ActionPoints.SpawnEntityFunction
//  * ActionPoints.ManualEnemyProjectile
//  * Entity.BaseEntityRomStruct.initialProjectileId
//  * Entity.SpawnManualEnemyProjectiles
//  * Entity.DeleteIfOutsideAndGotoNextEntity
//  * Entity.CountEntitiesInList
//  * Entity.MoveEntity() (Entity.MovementState.NO_GRAVITY_FLAG set)


namespace Entities {

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
    ProcessCollision:
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



namespace ActionPointSpawner {
    buildFunctionTable(BaseEntityFunctionTable, ActionPointSpawner)

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

        // always activate
        sec
        rts
    }

    a16()
    i16()
    code()
    Destructor:
    Deactivated:
    ProcessCollision:
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

        jmp     Entity.DrawAndGotoNextEntity
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
    Text.Console.SetCursor(0, 4)
    Text.Console.PrintConstString("Also tests:\n  DeleteAndGotoNextEntity")

    Text.Console.SetCursor(0, 7)
    Text.Console.PrintConstString("Press L/R to spawn\n  Manual Projectiles")

    jsr     EntityLoop.Init_NoRoom

    jsr     EnableDisplay_Full


    // Load the entity

    lda.w   #MetaTiles.map.LEFT + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #MetaTiles.map.TOP + Camera.DISPLAY_HEIGHT / 2
    sta.w   Entity.Spawn.yPos

    lda.w   #Project.EntityIds.ActionPointSpawner

    jsr     Entity.Spawn
    bcc     Fail


    // Process the Entity Loop
    // -----------------------

    GameLoop:
        jsr     EntityLoop.ProcessFrame

        PrintListSizes()

        jsr     WaitFrame_PrintFreeCycles

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.buttons
        beq     GameLoop


    jmp     EntityLoop.Cleanup

Fail:
    clc
    rts
}

}
}

// vim: ft=bass-65816 ts=4 sw=4 et:

