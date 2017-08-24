// Ensure that warnings do register similar colors

define MEMORY_MAP = HIROM
define ROM_SIZE = 1

include "../../../src/common/memory.inc"
include "../../../src/common/cpu.inc"

createCodeBlock(code,       0xc08000, 0xc0ffaf)
createRamBlock(shadow,      0x7e1100, 0x7e1f7f)

include "../../../src/errors/warnings.inc"

Warnings.Register(ONE,   18,  0, 18)
Warnings.Register(TEST,  18, 24, 31)
Warnings.Register(TWO,    0,  0, 18)
Warnings.Register(THREE,  0, 18, 31)

Warnings.Register(DUP,   12, 20, 28) // ERROR

// vim: ft=bass-65816 ts=4 sw=4 et:

