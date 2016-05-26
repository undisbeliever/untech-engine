arch snes.cpu

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)
define ROM_SPEED(slow)
define REGION(Australia)
define ROM_NAME("UNTECH UNIT TESTS")
define VERSION(0)

include "../../src/common.inc"

createCodeBlock(code,  0xc08000, 0xc0ffaf)

createDataBlock(rom1,  0xc10000, 0xc1ffff)

createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)


include "../../src/interrupts.inc"


code(code)
CopHandler:
IrqHandler:
EmptyHandler:
NmiHandler:
    rti


a8()
i16()
scope Main: {
    // just test that brk works as expected
    brk     #0
}

// vim: ft=asm ts=4 sw=4 et:

