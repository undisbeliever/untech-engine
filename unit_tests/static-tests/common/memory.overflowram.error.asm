// Overflow a RAM block

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"

createRamBlock(zeropage, 0x000000, 0x0000ff)
createRamBlock(lowram,   0x7e0100, 0x7e1f7f)
createRamBlock(stack,    0x7e1f80, 0x7e1fff)

allocate(zpVar1, zeropage, 0x80)

namespace innerScope {
    allocate(zpVar1, zeropage, 0x80)
}

allocate(over, zeropage, 1) // ERROR

