
define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"

createCodeBlock(code,  0xc08000, 0xc0ffaf)
createDataBlock(rom1,  0xc18000, 0xc1ffff)
createDataBlock(rom2,  0xc28000, 0xc2ffff)

createRamBlock(dp,     0x000000, 0x0000ff)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)
createRamBlock(shadow, 0x7e0100, 0x7e1f7f)

finalizeMemory()

createRamBlock(wram7e, 0x7e2000, 0x7effff) // ERROR

