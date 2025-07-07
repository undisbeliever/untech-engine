// Allocate block inside Low-RAM

define MEMORY_MAP = HIROM
define ROM_SIZE = 32

include "../../../engine/common/memory.inc"

createCodeBlock(code,  0x800000, 0x802000) // ERROR

