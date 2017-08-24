// Allocate block outside the ROM area

define MEMORY_MAP = LOROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"

createCodeBlock(code,  0x002000, 0x00ffaf) // ERROR

