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



createCodeBlock(code,                   0x808000, 0x80ff8f)
createCodeBlock(interrupt_code,         0x80ff90, 0x80ffaf)

createDataBlock(farCode,                0x818000, 0x81dfff)
createDataBlock(breakIdStrings,         0x81e000, 0x81ffff)

createDataBlock(TadData0,               0x828000, 0x82ffff)

createDataBlock(RES_Block0,             0x838000, 0x83ffff)
createDataBlock(RES_Block1,             0x848000, 0x84ffff)
createDataBlock(RES_Block2,             0x858000, 0x85ffff)
createDataBlock(RES_Block3,             0x868000, 0x86ffff)
createDataBlock(RES_Block4,             0x878000, 0x87ffff)


createRamBlock(zeropage,    0x000000, 0x0000ff)
createRamBlock(entityBlock, 0x7e0100, 0x7e10ff)
createRamBlock(lowram,      0x7e1100, 0x7e1f7f)
createRamBlock(stack,       0x7e1f80, 0x7e1fff)
createRamBlock(wram7e,      0x7e2000, 0x7effff)
createRamBlock(wram7f,      0x7f0000, 0x7fffff)


// Store small rodata immediately after audio data
// Requires `gen/audio-data.inc` to be loaded first
define DEFAULT_RODATA_BLOCK = TadData0
define Text.Console.StringBlock = TadData0

// Allocate 2 pages (512 bytes) to store the game state
constant N_GAME_STATE_PAGES = 2


// Temporary Variables
// ===================
evaluate n = 0
while {n} < 10 {
    allocate(zpTmp{n}, zeropage, 2)
    evaluate n = {n} + 1
}


// Entity Layout
// =============
namespace Entity {
    constant ENTITY_SIZE = 64
    constant N_ENTITIES = 63
}

define PLAYER_PROJECTILE_ENTITY_LIST = playerProjectiles
define ENEMY_PROJECTILE_ENTITY_LIST  = enemyProjectiles

