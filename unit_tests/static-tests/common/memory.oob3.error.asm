// Allocate block outside the ROM_SIZE area

define MEMORY_MAP(HIROM)
define ROM_SIZE(16)

include "../../../src/common/memory.inc"

createDataBlock(data,  0xe00000, 0xe0ffff) // ERROR

