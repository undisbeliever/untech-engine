// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


define MEMORY_MAP = HIROM
define ROM_SIZE = 1
define ROM_SPEED = slow
define REGION = US
define ROM_NAME = "UNTECH UNIT TESTS"
define VERSION = 0

// count number of free cycles in WaitFrame instead of sleeping
define COUNT_FREE_CYCLES


// Adds more break on error tests to the engine
// ::TODO write more code that uses this variable::
define DEBUG_BUILD



include "../../src/common.inc"
include "memmap.inc"

include "resources.inc"

include "../gen/tables/entityhitbox-collisionorder.inc"
include "../gen/tables/sine-table.inc"

include "../../src/untech.inc"

include "includes/test-framework.inc"
include "includes/utils.inc"
include "includes/gameloop.inc"
include "includes/blank-player.inc"


// A simple test that always succeeds
a16()
i16()
code()
Test.add("Success Test")
function SuccessTest {
    sec
    rts
}

include "actionpoints.inc"
include "bytecodes.inc"
include "scenes.inc"

include "tests/dma/obj-palette.inc"
include "tests/dma/tile16.inc"

include "tests/entity/entities/blank.inc"
include "tests/entity/entities/changetoparticle.inc"
include "tests/entity/entities/deleteafterdelay.inc"
include "tests/entity/entities/deactivate-if-outside.inc"
include "tests/entity/entities/delete-if-outside.inc"
include "tests/entity/entities/delete-if-animation-ends.inc"
include "tests/entity/entities/spawn-and-change-list.inc"

include "tests/entity/_common.inc"
include "tests/entity/allocation.inc"
include "tests/entity/counters.inc"
include "tests/entity/deallocation.inc"
include "tests/entity/gameloop.inc"
include "tests/entity/instanceidtable.inc"
include "tests/entity/spawn.inc"
include "tests/entity/transitions.inc"

include "tests/math/_common.inc"
include "tests/math/division.inc"
include "tests/math/macros.inc"
include "tests/math/multiplication.inc"

include "tests/metasprite/_common.inc"
include "tests/metasprite/palette.inc"
include "tests/metasprite/vram.inc"
include "tests/metasprite/render.inc"
include "tests/metasprite/animation.inc"

include "tests/vblank/vblank.inc"

include "tests/text/string.inc"

include "tests/util/lz4.inc"

include "tests/scripting/game-state.inc"
include "tests/scripting/scripting.inc"


include "interactive/script-trigger-test.inc"
include "interactive/spawn-entity-group.inc"
include "interactive/interactive-tiles-test.inc"
include "interactive/tile-collision-test.inc"
include "interactive/entity-validation.inc"
include "interactive/entity-collisions.inc"
include "interactive/metatiles-render.inc"
include "interactive/entity-metasprite.inc"
include "interactive/entity-actionpointspawner.inc"
include "interactive/hdma-circular-window.inc"
include "interactive/hdma-horizontal-trapezium-window.inc"
include "interactive/hdma-rotated-triangular-window.inc"
include "interactive/hdma-triangular-window.inc"
include "interactive/errors.inc"


constant Main = Test.ProcessTests



Test.finalizeTable()

finalizeMemory()

// vim: ft=bass-65816 ts=4 sw=4 et:

