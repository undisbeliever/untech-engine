// Test RAM blocks

define MEMORY_MAP(LOROM)
define ROM_SIZE(1)

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"

createRamBlock(dp,     0x000000, 0x0000ff)
createRamBlock(shadow, 0x7e0100, 0x7e1f7f)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)

assert({::blocks.stack.size} == 0)
assert({::blocks.stack.pos} == 0x7e1f80)
assert({::blocks.stack.end} == 0x7e1fff)

allocate(dptmp1, dp, 2)
allocate(dptmp2, dp, 2)
allocate(dptmp3, dp, 2)

assert({::blocks.dp.pos} == 6)
assert({::blocks.dp.size} == 6)
assert({::blocks.dp.remaining} == 0x100 - 6)

constant c(8)

allocate(var1, shadow, 0x100)

scope testInScope {
    allocate(var2, shadow, 0x020 * 2)
    allocate(var3, shadow, c * 4)
}

assert({::blocks.shadow.size} == 0x160)
assert({::blocks.shadow.pos} == 0x7e0260)
assert({::blocks.shadow.remaining} == 0x1e80 - 0x160)

// vim: ft=bass-65816 ts=4 sw=4 et:

