// Allocate block outside the ROM area

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)

include "../../../src/common/memory.inc"

createCodeBlock(rom0,  0xc00000, 0xc0ffff) // ERROR

