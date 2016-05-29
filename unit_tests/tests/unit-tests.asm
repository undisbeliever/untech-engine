arch snes.cpu

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)
define ROM_SPEED(slow)
define REGION(Australia)
define ROM_NAME("UNTECH UNIT TESTS")
define VERSION(0)

include "../../src/common.inc"

createCodeBlock(code,                       0xc08000, 0xc0ffaf)
createDataBlock(testTableBlock,             0xc00000, 0xc07fff)

createDataBlock(DMA_Tile16Data,             0xc10000, 0xc10fff)

createDataBlock(rom0,                       0xc11000, 0xc14000)
createDataBlock(testNameBlock,              0xc1a000, 0xc1ffff)


createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)
createRamBlock(wram7e, 0x7e2000, 0x7effff)


include "../../src/dma.inc"
include "../../src/interrupts.inc"
include "../../src/math.inc"


include "includes/test-framework.inc"

// A simple test that always succeeds
code()
Test.add("Success Test")
scope SuccessTest: {
    sec
    rts
}

include "tests/dma.inc"
include "tests/math.inc"

Test.finalizeTable()

code(code)
CopHandler:
IrqHandler:
EmptyHandler:
    rti

// vim: ft=asm ts=4 sw=4 et:

