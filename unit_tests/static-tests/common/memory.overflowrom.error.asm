// Allocate block outside the ROM area

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)

include "../../../src/common/memory.inc"

createDataBlock(rom0,  0xc00000, 0xc000ff)
createDataBlock(rom1,  0xc00100, 0xc01000)

rodata(rom0)
    fill 0x101, 0

rodata(rom1) // ERROR

