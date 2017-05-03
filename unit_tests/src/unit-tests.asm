arch snes.cpu

define MEMORY_MAP(HIROM)
define ROM_SIZE(1)
define ROM_SPEED(slow)
define REGION(Australia)
define ROM_NAME("UNTECH UNIT TESTS")
define VERSION(0)

include "../../src/common.inc"
include "memmap.inc"

include "../gen/metasprites.inc"
include "../resources/text/text.inc"

include "../gen/tables/entityhitbox-collisionorder.inc"

include "../../src/dma.inc"
include "../../src/hdma.inc"
include "../../src/interrupts.inc"
include "../../src/math.inc"
include "../../src/metasprite.inc"
include "../../src/text.inc"

include "../../src/camera.inc"
include "../../src/entity.inc"

include "includes/test-framework.inc"
include "includes/entityloop.inc"


// A simple test that always succeeds
a16()
i16()
code()
Test.add("Success Test")
scope SuccessTest: {
    sec
    rts
}

include "tests/dma.inc"
include "tests/entity.inc"
include "tests/math.inc"
include "tests/metasprite.inc"
include "tests/text.inc"

include "interactive/entity-metasprite.inc"
include "interactive/entity-actionpointspawner.inc"
include "interactive/entityhitbox-collisions.inc"
include "interactive/hdma-circular-window.inc"


constant Main(Test.ProcessTests)


Test.finalizeTable()


au()
iu()
code(code)
CopHandler:
IrqHandler:
EmptyHandler:
    rti


finalizeMemory()

// vim: ft=bass-65816 ts=4 sw=4 et:

