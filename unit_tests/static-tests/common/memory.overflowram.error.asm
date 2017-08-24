// Overflow a RAM block

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"

createRamBlock(dp,     0x000000, 0x0000ff)
createRamBlock(shadow, 0x7e0100, 0x7e1f7f)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)

allocate(dpVar1, dp, 0x80)

namespace innerScope {
    allocate(dpVar1, dp, 0x80)
}

allocate(over, dp, 1) // ERROR

