
.PHONY: all
all: static-tests


BINARY         := bin/unit-tests.sfc
MAIN           := src/unit-tests.asm

METASPRITE_PRO := resources/metasprite/metasprites.utmspro
RESOURCES_PRO  := resources/resources.utres

SRC_DIRS       := src resources
UNTECH_DIR     := ..

RESOURCES      := gen/font-fixed.1bpp gen/lz4-test/font-fixed.1bpp.lz4b \
                  gen/lz4-test/1kzeros.lz4b gen/lz4-test/incompressable.lz4b

STATIC_TESTS   := $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)


include ../GNUmakefile.in


.PHONY: static-tests
static-tests: $(STATIC_TESTS)

static-tests/%.success.asm: bass FORCE
	$(bass) -strict $@

static-tests/%.error.asm: bass FORCE
	python3 static-tests/errortest.py -b $(bass) $@

FORCE:


gen/font-fixed.1bpp: resources/text/font-fixed.png
	$(untech-png2tileset) -b 1 -o "$@" "$<"

gen/lz4-test/font-fixed.1bpp.lz4b: gen/font-fixed.1bpp
	$(untech-lz4c) -v -o "$@" "$<"

gen/lz4-test/%.lz4b: gen/lz4-test/%.bin
	$(untech-lz4c) -v -o "$@" "$<"

gen/lz4-test/%.bin: resources/lz4-test/%.asm
	$(bass) -strict -o "$@" "$<"

