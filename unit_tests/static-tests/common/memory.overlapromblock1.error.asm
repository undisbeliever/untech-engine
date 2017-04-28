// Try and create two Data blocks that overlap each other

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)

include "../../../src/common/memory.inc"

createDataBlock(rom0,  0xc00000, 0xc000ff)
createDataBlock(rom1,  0xc00100, 0xc001ff)
createDataBlock(rom2,  0xc00100, 0xc0017f)  // ERROR
createDataBlock(rom3,  0xc10000, 0xc1ffff)

