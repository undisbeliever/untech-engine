// Test RAM blocks

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/assert.inc"
include "../../../src/common/memory.inc"

createRamBlock(dp,     0x000000, 0x0000ff)
createRamBlock(lowram, 0x7e0100, 0x7e1f7f)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)

assert(__MEMORY__.ramBlocks.stack.size == 0)
assert(__MEMORY__.ramBlocks.stack.pos == 0x7e1f80)
assert(__MEMORY__.ramBlocks.stack.end == 0x7e1fff)

allocate(dptmp1, dp, 2)
allocate(dptmp2, dp, 2)
allocate(dptmp3, dp, 2)

assert(__MEMORY__.ramBlocks.dp.pos == 6)
assert(__MEMORY__.ramBlocks.dp.size == 6)
assert(__MEMORY__.ramBlocks.dp.remaining == 0x100 - 6)

constant c = 8

allocate(var1, lowram, 0x100)

namespace testInScope {
    allocate(var2, lowram, 0x020 * 2)
    allocate(var3, lowram, c * 4)
}

assert(__MEMORY__.ramBlocks.lowram.size == 0x160)
assert(__MEMORY__.ramBlocks.lowram.pos == 0x7e0260)
assert(__MEMORY__.ramBlocks.lowram.remaining == 0x1e80 - 0x160)

// vim: ft=bass-65816 ts=4 sw=4 et:

