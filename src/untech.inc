
include "dma/forceblank.inc"
include "dma/metasprite.inc"
include "dma/palette.inc"
include "dma/text.inc"
include "dma/tile16.inc"
include "dma/_common.inc"

include "hdma/_variables.inc"
include "hdma/circular-window.inc"
include "hdma/_common.inc"

include "interrupts/reset_handler.inc"
include "interrupts/break_handler.inc"
include "interrupts/nmi/controller.inc"
include "interrupts/nmi/scrollbuffer.inc"
include "interrupts/nmi/framecounter.inc"
include "interrupts/nmi_handler.inc"


include "math/_variables.inc"
include "math/_macros.inc"
include "math/division.inc"
include "math/multiplication.inc"

include "text/_variables.inc"
include "text/console.inc"
include "text/string.inc"


include "metasprite/_dataformat.inc"
include "metasprite/_variables.inc"
include "metasprite/animation.inc"
include "metasprite/entityhitbox.inc"
include "metasprite/entityhitbox-collisions.inc"
include "metasprite/palette.inc"
include "metasprite/vram.inc"
include "metasprite/render.inc"
include "metasprite/_common.inc"

include "camera/_variables.inc"
include "camera/_common.inc"
include "camera/activewindow.inc"

include "entity/_dataformat.inc"
include "entity/_variables.inc"
include "entity/allocation.inc"
include "entity/counters.inc"
include "entity/deallocation.inc"
include "entity/gameloop.inc"
include "entity/render.inc"
include "entity/spawn.inc"

// vim: ft=bass-65816 ts=4 sw=4 et:
