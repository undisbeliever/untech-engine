
scope InteractiveTests {
scope EntityHitboxCollisionsTest {

// Tests:
//  * EntityLoop
//  * MetaSprite EntityHitbox Collisions


scope PlayerEntity {
    buildFunctionTable(BaseEntityFunctionTable, PlayerEntity)
    Entity.useDefaultRenderer()

    constant N_FRAMES(8)

    scope EntityStruct {
        childstruct(BaseEntity)
            field(selectedFrame, 2)
        endstruct()
    }

    a16()
    i16()
    code()
    scope Init: {
        stz.b   EntityStruct.selectedFrame

        lda.w   #0
        jmp     MetaSprite.SetFrame
    }

    a16()
    i16()
    code()
    Destructor: {
        Text.Console.PrintConstString("Player.Destructor\n")
        rts
    }

    a16()
    i16()
    code()
    Process: {
        lda.w   Controller.Joy1.current
        bit.w   #JOY.left
        beq     +
            dec.b   EntityStruct.xPos.px
        +
        bit.w   #JOY.right
        beq     +
            inc.b   EntityStruct.xPos.px
        +
        bit.w   #JOY.up
        beq     +
            dec.b   EntityStruct.yPos.px
        +
        bit.w   #JOY.down
        beq     +
            inc.b   EntityStruct.yPos.px
        +

        lda.w   Controller.Joy1.pressed
        and.w   #JOY.select
        beq     EndIf
            lda.b   EntityStruct.selectedFrame
            inc
            cmp.w   #N_FRAMES
            bcc     +
                lda.w   #0
            +
            sta.b   EntityStruct.selectedFrame
            jsr     MetaSprite.SetFrame

        EndIf:

        rts
    }

    scope CalculateAttackValue: {
        bcc     +
            Text.Console.PrintConstString("P.CalculateAttackValue (s)\n")
            rts
        +
            Text.Console.PrintConstString("P.CalculateAttackValue (n)\n")
            rts
    }

    a16()
    i16()
    code()
    scope TakeDamageFromEntity: {
        bcc     +
            Text.Console.PrintConstString("P.TakeDamageFromEntity (wk)\n")
            rts
        +
            Text.Console.PrintConstString("P.TakeDamageFromEntity (n)\n")
            rts
        rts
    }

    scope BlockWithShield: {
        bcc     +
            Text.Console.PrintConstString("P.BlockWithShield (attack)\n")
            rts
        +
            Text.Console.PrintConstString("P.BlockWithShield (body)\n")
            rts
    }

    scope CollideWithBody: {
        Text.Console.PrintConstString("P.CollideWithBody\n")
        rts
    }

    scope CollideWithShield: {
        Text.Console.PrintConstString("P.CollideWithShield\n")
        rts
    }

    rodata(EN_RomData)
    scope RomData: {
        dw  FunctionTable
        db  Entity.ListId.PLAYER        // initial List
        dw  MSFS.EhCollisionTest_Player // frameSetId
        db  0                           // defaultPalette
    }

    rodata(EN_EntityList)
    constant ENTITY_ID((pc() - Entity.Data.EntityList) / 2)
        dw  RomData
}


scope EnemyEntity {
    buildFunctionTable(BaseEntityFunctionTable, EnemyEntity)
    Entity.useDefaultRenderer()

    // IN: A = frameId
    a16()
    i16()
    code()
    scope Init: {
        jmp     MetaSprite.SetFrame
    }

    a16()
    i16()
    code()
    Destructor: {
        Text.Console.PrintConstString("E.Destructor\n")
        rts
    }

    a16()
    i16()
    code()
    Process: {
        rts
    }

    scope CalculateAttackValue: {
        bcc     +
            Text.Console.PrintConstString("E.CalculateAttackValue (s)\n")
            rts
        +
            Text.Console.PrintConstString("E.CalculateAttackValue (n)\n")
            rts
    }

    a16()
    i16()
    code()
    scope TakeDamageFromEntity: {
        bcc     +
            Text.Console.PrintConstString("E.TakeDamageFromEntity (wk)\n")
            rts
        +
            Text.Console.PrintConstString("E.TakeDamageFromEntity (n)\n")
            rts
        rts
    }

    scope BlockWithShield: {
        bcc     +
            Text.Console.PrintConstString("E.BlockWithShield (attack)\n")
            rts
        +
            Text.Console.PrintConstString("E.BlockWithShield (body)\n")
            rts
    }

    scope CollideWithBody: {
        Text.Console.PrintConstString("E.CollideWithBody\n")
        rts
    }

    scope CollideWithShield: {
        Text.Console.PrintConstString("E.CollideWithShield\n")
        rts
    }

    rodata(EN_RomData)
    scope RomData: {
        dw  FunctionTable
        db  Entity.ListId.ENEMY         // initial List
        dw  MSFS.EhCollisionTest_Enemy  // frameSetId
        db  0                           // defaultPalette
    }

    rodata(EN_EntityList)
    constant ENTITY_ID((pc() - Entity.Data.EntityList) / 2)
        dw  RomData
}

rodata(rom0)
scope EnemyTable: {
constant Y_DELTA(20)
constant N_COLUMNS(4)
constant Y_BOTTOM(Camera.ORIGIN + Camera.DISPLAY_HEIGHT - 30)

constant X_LEFT(Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2 - 10)
constant X_RIGHT(Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2 + 10)

    struct(pc())
        field(xPos, 2)
        field(yPos, 2)
        field(parameter, 2)
    endstruct()

    //  xPos,       yPos,                   frameId
    dw  X_LEFT,     Y_BOTTOM - Y_DELTA * 2,  MSEO.EhCollisionTest_Enemy.Frames.box
    dw  X_RIGHT,    Y_BOTTOM - Y_DELTA * 3,  MSEO.EhCollisionTest_Enemy.Frames.row1
    dw  X_RIGHT,    Y_BOTTOM - Y_DELTA * 2,  MSEO.EhCollisionTest_Enemy.Frames.row2
    dw  X_RIGHT,    Y_BOTTOM - Y_DELTA * 1,  MSEO.EhCollisionTest_Enemy.Frames.row3
    dw  X_RIGHT,    Y_BOTTOM - Y_DELTA * 0,  MSEO.EhCollisionTest_Enemy.Frames.row4

    constant N_ENEMIES((pc() - EnemyTable) / size)
}


// The Test
a16()
i16()
code()
Test.add("EntityHitbox Collisions Tst")
scope Test: {
constant tmp_index(Test.tmp)

    jsr     EntityLoop.Init

    // Load the entities

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_WIDTH / 2
    sta.w   Entity.Spawn.xPos

    lda.w   #Camera.ORIGIN + Camera.DISPLAY_HEIGHT * 2 / 3
    sta.w   Entity.Spawn.yPos

    lda.w   #PlayerEntity.ENTITY_ID

    jsr     Entity.Spawn
    bcc     Fail


    ldx.w   #0
    SpawnEnemyLoop:
        stx.w   tmp_index

        lda.l   EnemyTable.xPos,x
        sta.w   Entity.Spawn.xPos

        lda.l   EnemyTable.yPos,x
        sta.w   Entity.Spawn.yPos

        lda.l   EnemyTable.parameter,x
        tay

        lda.w   #EnemyEntity.ENTITY_ID

        jsr     Entity.Spawn
        bcc     Fail

        lda.w   tmp_index
        clc
        adc.w   #EnemyTable.size
        tax
        cpx.w   #EnemyTable.size * EnemyTable.N_ENEMIES
        bcc     SpawnEnemyLoop


    // Process the Entity Loop
    // -----------------------

    GameLoop:
        jsr     Text.Console.ClearBuffer
        Text.Console.PrintConstString("EntityHitbox Collision Test")
        Text.Console.SetCursor(0, 2)
        Text.Console.PrintConstString("Use D-Pad to move Player\nSelect changes hitbox type")

        Text.Console.SetCursor(0, 7)

        jsr     EntityLoop.ProcessFrame

        jsr     Dma.TransferOnNextVBlank
        jsr     WaitFrame

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

