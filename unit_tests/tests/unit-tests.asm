arch snes.cpu

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)
define ROM_SPEED(slow)
define REGION(Australia)
define ROM_NAME("UNTECH UNIT TESTS")
define VERSION(0)

include "../../src/common.inc"

createCodeBlock(code,                   0xc08000, 0xc0ffaf)
createDataBlock(testTableBlock,         0xc00000, 0xc07fff)

createDataBlock(MS_FrameSetList,        0xc10000, 0xc100ff)
createDataBlock(MS_PaletteList,         0xc10100, 0xc101ff)
createDataBlock(MS_FrameList,           0xc10200, 0xc102ff)
createDataBlock(MS_AnimationList,       0xc10300, 0xc103ff)

createDataBlock(MS_FrameSetData,        0xc10400, 0xc104ff)
createDataBlock(MS_FrameData,           0xc10500, 0xc105ff)
createDataBlock(MS_FrameObjectsData,    0xc10600, 0xc106ff)
createDataBlock(MS_TileHitboxData,      0xc10700, 0xc107ff)
createDataBlock(MS_EntityHitboxData,    0xc10800, 0xc108ff)
createDataBlock(MS_ActionPointsData,    0xc10900, 0xc109ff)

createDataBlock(MS_PaletteData,         0xc10a00, 0xc10eff)

createDataBlock(DMA_Tile16Data,         0xc10f00, 0xc10fff)

createDataBlock(rom0,                   0xc11000, 0xc14000)
createDataBlock(testNameBlock,          0xc1a000, 0xc1ffff)


createRamBlock(dp,     0x000000, 0x000100)
createRamBlock(shadow, 0x7e0100, 0x7e1f80)
createRamBlock(stack,  0x7e1f80, 0x7e1fff)
createRamBlock(wram7e, 0x7e2000, 0x7effff)



include "../../src/dma.inc"
include "../../src/interrupts.inc"
include "../../src/math.inc"
include "../../src/metasprite.inc"


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

