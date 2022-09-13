// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


createDataBlock(breakIdStrings,         0xc00000, 0xc004ff)
createDataBlock(testNameBlock,          0xc00500, 0xc01fff)
createDataBlock(testTableBlock,         0xc02000, 0xc021ff)

createDataBlock(rom0,                   0xc02200, 0xc059ff)

createCodeBlock(code,                   0xc05a00, 0xc0ff8f)
createCodeBlock(interrupt_code,         0xc0ff90, 0xc0ffaf)

createDataBlock(RES_Block0,             0xc10000, 0xc1ffff)


createRamBlock(dp,          0x000000, 0x0000ff)
createRamBlock(entityBlock, 0x7e0100, 0x7e10ff)
createRamBlock(lowram,      0x7e1100, 0x7e1f7f)
createRamBlock(stack,       0x7e1f80, 0x7e1fff)
createRamBlock(wram7e,      0x7e2000, 0x7effff)
createRamBlock(wram7f,      0x7f0000, 0x7fffff)


// Allocate 2 pages (512 bytes) to store the game state
constant N_GAME_STATE_PAGES = 2


// Entity Layout
// =============
namespace Entity {
    constant ENTITY_SIZE = 64
    constant N_ENTITIES = 63
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

// vim: ft=bass-65816 ts=4 sw=4 et:

