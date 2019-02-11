// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2017, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT

createCodeBlock(code,                   0xc08000, 0xc0ffaf)
createDataBlock(breakIdStrings,         0xc00000, 0xc017ff)
createDataBlock(testTableBlock,         0xc02000, 0xc03fff)

createDataBlock(RES_Lists,              0xc04000, 0xc04fff)
createDataBlock(rom0,                   0xc05000, 0xc07fff)

createDataBlock(RES_Block0,             0xc10000, 0xc11fff)
createDataBlock(RES_Block1,             0xc12000, 0xc13fff)
createDataBlock(RES_Block2,             0xc14000, 0xc15fff)
createDataBlock(RES_Block3,             0xc16000, 0xc17fff)
createDataBlock(RES_Block4,             0xc18000, 0xc19fff)
createDataBlock(RES_Block5,             0xc1a000, 0xc1bfff)

createDataBlock(EN_EntityDataList,      0xc1c000, 0xc1c07f)
createDataBlock(EN_ProjectileList,      0xc1c080, 0xc1c0ff)
createDataBlock(EN_RomData,             0xc1c100, 0xc1cfff)

createDataBlock(testNameBlock,          0xc1f000, 0xc1ffff)


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

namespace EntityLists {
    createEnum()
        // no collisions
        enum(Particles)

        // collides with: enemies
        enum(PlayerProjectiles)

        // collides with: player
        enum(EnemyProjectiles)

        // no collisions
        enum(Platforms)

        // collides with: enemies, enemy projectiles
        enum(Players)

        // collides with: player
        enum(Enemies)
    endEnum()
}


define PLAYER_PROJECTILE_ENTITY_LIST = PlayerProjectiles
define ENEMY_PROJECTILE_ENTITY_LIST  = EnemyProjectiles


// MetaTile engine settings
// ========================
constant MetaTiles.MAP_SIZE = 8192
constant MetaTiles.N_META_TILES = 512



// VRAM Layout
// ===========
constant VRAM_OBJ_WADDR = 0x6000
constant VRAM_CONSOLE_TILES_WADDR = 0x1000
constant VRAM_CONSOLE_MAP_WADDR = 0x0000

constant METATILES_MAP_WADDR = 0x0400   // 64x32 tiles in size
constant METATILES_TILES_WADDR = 0x2000

// vim: ft=bass-65816 ts=4 sw=4 et:

