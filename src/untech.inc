// This file is part of the UnTech Game Engine.
// Copyright (c) 2016 - 2021, Marcus Rowe <undisbeliever@gmail.com>.
// Distributed under The MIT License: https://opensource.org/licenses/MIT


architecture wdc65816-strict


constant UNTECH_VERSION_ID = 29


include "util/_dptmp.inc"

include "errors/break.inc"
include "errors/warnings.inc"

include "dma/_transferscale.inc"
include "dma/forceblank.inc"
include "dma/metasprite.inc"
include "dma/metatiles.inc"
include "dma/obj-palette.inc"
include "dma/resources.inc"
include "dma/text.inc"
include "dma/tile16.inc"
include "dma/_common.inc"

include "math/_macros.inc"

include "scripting/_variables-gamestate.inc"
include "metatiles/_variables.inc"
include "room/_variables.inc"

include "scripting/_dataformat.inc"
include "entity/_dataformat.inc"
include "metasprite/_dataformat.inc"
include "metatiles/_dataformat.inc"
include "resources/_dataformat.inc"
include "room/_dataformat.inc"

include "scripting/_variables.inc"
include "resources/_variables.inc"
include "hdma/_variables.inc"
include "math/_variables.inc"
include "text/_variables.inc"
include "metasprite/_variables.inc"
include "entity/_variables.inc"
include "camera/_variables.inc"


include "hdma/circular-window.inc"
include "hdma/_window-buffer.inc"
include "hdma/horizontal-trapezium-window.inc"
include "hdma/triangular-window.inc"
include "hdma/_common.inc"

include "interrupts/unused_interrupts.inc"
include "interrupts/break_handler.inc"
include "interrupts/reset_handler.inc"
include "interrupts/nmi_handler.inc"

include "math/division.inc"
include "math/multiplication.inc"

include "text/console.inc"
include "text/string.inc"

include "util/lz4.inc"

include "camera/_common.inc"
include "camera/center-on-entity.inc"

include "metasprite/animation.inc"
include "metasprite/collisionboxes.inc"
include "metasprite/palette.inc"
include "metasprite/vram.inc"
include "metasprite/render.inc"
include "metasprite/_common.inc"

include "entity/instanceidtable.inc"
include "entity/player.inc"

include "entity/activewindow.inc"
include "entity/allocation.inc"
include "entity/counters.inc"
include "entity/deallocation.inc"
include "entity/gameloop.inc"
include "entity/invalid-entity.inc"
include "entity/movement.inc"
include "entity/spawn.inc"
include "entity/transitions.inc"
include "entity/validation.inc"

include "metatiles/_mapheight.inc"
include "metatiles/collisions.inc"
include "metatiles/interactive-tiles.inc"
include "metatiles/render.inc"
include "metatiles/crumbling-tiles.inc"
include "metatiles/tileset.inc"
include "metatiles/_common.inc"
include "metatiles/_vblank.inc"

include "metasprite/actionpoints.inc"

include "resources/scene-layout.inc"
include "resources/animated-tileset.inc"
include "resources/images.inc"
include "resources/palette.inc"
include "resources/_common.inc"
include "resources/_vblank.inc"

include "scripting/game-state.inc"
include "scripting/background-tasks.inc"
include "scripting/scripts.inc"
include "scripting/bytecodes.inc"

include "room/maps.inc"
include "room/entrances.inc"
include "room/entity-groups.inc"
include "room/room-scripts.inc"
include "room/script-triggers.inc"
include "room/_common.inc"

include "vblank/controller.inc"
include "vblank/_common.inc"

include "entity/macros.inc"

// vim: ft=bass-65816 ts=4 sw=4 et:

