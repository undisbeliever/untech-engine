// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2019, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


createDataBlock(breakIdStrings,         0xc00000, 0xc007ff)
createDataBlock(testNameBlock,          0xc00800, 0xc02bff)
createDataBlock(testTableBlock,         0xc02c00, 0xc02fff)
createCodeBlock(code,                   0xc03000, 0xc0ff8f)
createCodeBlock(interrupt_code,         0xc0ff90, 0xc0ffaf)

createDataBlock(RES_Block0,             0xc10000, 0xc11fff)
createDataBlock(RES_Block1,             0xc12000, 0xc13fff)
createDataBlock(RES_Block2,             0xc14000, 0xc15fff)
createDataBlock(RES_Block3,             0xc16000, 0xc17fff)
createDataBlock(RES_Block4,             0xc18000, 0xc19fff)

createDataBlock(RES_Lists,              0xc1a000, 0xc1bfff)
createDataBlock(rom0,                   0xc1c000, 0xc1ffff)


createRamBlock(dp,          0x000000, 0x0000ff)
createRamBlock(entityBlock, 0x7e0100, 0x7e10ff)
createRamBlock(shadow,      0x7e1100, 0x7e1f7f)
createRamBlock(stack,       0x7e1f80, 0x7e1fff)
createRamBlock(wram7e,      0x7e2000, 0x7effff)
createRamBlock(wram7f,      0x7f0000, 0x7fffff)



// Entity Layout
// =============
namespace Entity {
    constant ENTITY_SIZE = 64
    constant N_ENTITIES = 64
}

// Allow tests to access entity pool
namespace EntityPool {
    evaluate n = 0
    while {n} < Entity.N_ENTITIES {
        constant entity{n} = Entity.entityPool + {n} * Entity.ENTITY_SIZE
        evaluate n = {n} + 1
    }
}

define PLAYER_PROJECTILE_ENTITY_LIST = PlayerProjectiles
define ENEMY_PROJECTILE_ENTITY_LIST  = EnemyProjectiles


// MetaTile engine settings
// ========================
constant MetaTiles.MAP_SIZE = 16384

// vim: ft=bass-65816 ts=4 sw=4 et:

