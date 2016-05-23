// Allocate block inside shadow RAM

define MEMORY_MAP(HIROM)
define ROM_SIZE(32)

include "../../../src/common/memory.inc"

createCodeBlock(code,  0x800000, 0x802000)

