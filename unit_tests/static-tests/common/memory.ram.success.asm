// Test RAM blocks

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../engine/common/assert.inc"
include "../../../engine/common/memory.inc"

createRamBlock(zeropage, 0x000000, 0x0000ff)
createRamBlock(lowram,   0x7e0100, 0x7e1f7f)
createRamBlock(stack,    0x7e1f80, 0x7e1fff)

assert(__MEMORY__.ramBlocks.stack.size == 0)
assert(__MEMORY__.ramBlocks.stack.pos == 0x7e1f80)
assert(__MEMORY__.ramBlocks.stack.end == 0x7e1fff)

allocate(zptmp1, zeropage, 2)
allocate(zptmp2, zeropage, 2)
allocate(zptmp3, zeropage, 2)

assert(__MEMORY__.ramBlocks.zeropage.pos == 6)
assert(__MEMORY__.ramBlocks.zeropage.size == 6)
assert(__MEMORY__.ramBlocks.zeropage.remaining == 0x100 - 6)

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

