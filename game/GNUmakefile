
BINARY         := bin/rescue.sfc
DEBUG_BINARY   := bin/rescue-debug.sfc

BASS_FLAGS     := -d RELEASE_BUILD

MAIN           := src/_main.asm

MEMORY_MAP     := lorom

PROJECT_FILE   := resources/rescue.utproject
TAD_PROJECT    := resources/audio/rescue.terrificaudio

SRC_DIRS       := src resources
TABLE_DIRS     := tables
UNTECH_DIR     := ..

GEN_DIR        := gen
RESOURCES      := gen/font-fixed.2bpp


RESOURCES      += gen/movement-table.inc


include ../engine/GNUmakefile.in


all: bin/rescue-debug.sfc

DEBUG_SYMBOLS := $(DEBUG_BINARY:.sfc=.sym)

$(DEBUG_BINARY): $(call rwildcard_all, $(SRC_DIRS) $(UNTECH_DIR)/engine)
$(DEBUG_BINARY): $(TABLE_INCS)
$(DEBUG_BINARY): $(RESOURCES)

$(DEBUG_BINARY) $(DEBUG_SYMBOLS): $(MAIN)
    ifeq ($(MEMORY_MAP), hirom)
	$(bass) -d DEBUG_BUILD -strict -o $(DEBUG_BINARY) -sym $(DEBUG_BINARY:.sfc=.sym) $(MAIN)
	$(untech-write-sfc-checksum) --hirom $(DEBUG_BINARY)
    else ifeq ($(MEMORY_MAP), lorom)
	$(bass) -d DEBUG_BUILD -strict -o $(DEBUG_BINARY) -sym $(DEBUG_BINARY:.sfc=.sym) $(MAIN)
	$(untech-write-sfc-checksum) --lorom $(DEBUG_BINARY)
    else
      $(error "Unknown memory map, please set MEMORY_MAP to 'hirom' or 'lorom'")
    endif


gen/movement-table.inc: resources/movement-table.csv tools/gen-movement-table.py
	python3 -bb tools/gen-movement-table.py -o "$@" resources/movement-table.csv


gen/font-fixed.2bpp: resources/text/font-fixed.png
	$(untech-png2tileset) -b 2 -o '$@' '$<'



.PHONY: clean-debug
clean: clean-debug
clean-debug:
	$(RM) $(DEBUG_BINARY) $(DEBUG_SYMBOLS)

