// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
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


namespace Entities {

namespace EightDirectionProjectile {
    buildFunctionTable(BaseEntityFunctionTable, EightDirectionProjectile)

    // Slow speed to exhaust entity pool.
    constant VELOCITY = 0x8000
    constant DIAGONAL_VELOCITY = 0x5a82 // sqrt(0x8000 ** 2 / 2

    namespace EntityStruct {
        childstruct(BaseEntity)
            field(tmp, 4)
        endstruct()
    }

    // OUT: C set = always activate entity
    a16()
    i16()
    code()
    function Init {
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
        beq     Orthogonal

        lda.w   MetaSprite.ActionPoint.positionPair.yPos
        cmp.b   #INT_MS8_OFFSET
        beq     Orthogonal

            // Projectile is Diagonal
            ldy.w   #DIAGONAL_VELOCITY
            sty.b   EntityStruct.tmp
            ldy.w   #DIAGONAL_VELOCITY >> 16
            bra     EndIf

        Orthogonal:
            // Projectile is Orthogonal
            ldy.w   #VELOCITY
            sty.b   EntityStruct.tmp
            ldy.w   #VELOCITY >> 16

    EndIf:
        sty.b   EntityStruct.tmp + 2


        rep     #$30
    a16()

        macro _SetVelocity(c) {
            namespace {c} {
                // Set Projectile Velocity
                lda.w   MetaSprite.ActionPoint.positionPair.{c}Pos
                if '{c}' == 'x' {
                    and.w   #0xff
                }
                cmp.w   #INT_MS8_OFFSET
                beq     Zero
                bcs     Positive

                Negative:
                    // load negative velocity
                    sec
                    lda.w   #0
                    sbc.b   EntityStruct.tmp
                    sta.b   EntityStruct.{c}Vecl

                    lda.w   #0
                    sbc.b   EntityStruct.tmp + 2
                    sta.b   EntityStruct.{c}Vecl + 2

                    bra     EndIf

                Positive:
                    lda.b   EntityStruct.tmp
                    sta.b   EntityStruct.{c}Vecl
                    lda.b   EntityStruct.tmp + 2
                    sta.b   EntityStruct.{c}Vecl + 2

                    bra     EndIf

                Zero:
                    stz.b   EntityStruct.{c}Vecl
                    stz.b   EntityStruct.{c}Vecl + 2

            EndIf:
            }
        }

        _SetVelocity(x)
        _SetVelocity(y)

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
        macro _addVelocity(c) {
            clc
            lda.b   BaseEntity.{c}Vecl
            adc.b   BaseEntity.{c}Pos.sx
            sta.b   BaseEntity.{c}Pos.sx

            lda.b   BaseEntity.{c}Vecl + 1
            xba
            bpl     +
                ora.w   #$ff00
                bra     ++
            +
                and.w   #$00ff
            +
            adc.b   BaseEntity.{c}Pos.px
            sta.b   BaseEntity.{c}Pos.px
        }
        _addVelocity(x)
        _addVelocity(y)

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

    jsr     EntityLoop.Init

    jsr     EnableDisplay_Full


    // Load the entity

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_HEIGHT / 2
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

        lda.l   JOY1
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

