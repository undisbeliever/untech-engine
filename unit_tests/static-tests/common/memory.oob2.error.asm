// Allocate block outside the ROM area

define MEMORY_MAP = HIROM
define ROM_SIZE = 1

include "../../../engine/common/memory.inc"

createCodeBlock(code,  0x802000, 0x80ffaf) // ERROR

