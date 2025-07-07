
BINARY         := bin/rescue.sfc
MAIN           := src/_main.asm

MEMORY_MAP     := lorom

PROJECT_FILE   := resources/rescue.utproject
TAD_PROJECT    := resources/audio/rescue.terrificaudio

SRC_DIRS       := src resources
TABLE_DIRS     := tables
UNTECH_DIR     := ..

GEN_DIR        := gen
RESOURCES      := gen/font-fixed.1bpp


RESOURCES      += gen/movement-table.inc


include ../engine/GNUmakefile.in


gen/movement-table.inc: resources/movement-table.csv tools/gen-movement-table.py
	python3 -bb tools/gen-movement-table.py -o "$@" resources/movement-table.csv


gen/font-fixed.1bpp: resources/text/font-fixed.png
	$(untech-png2tileset) -b 1 -o '$@' '$<'


