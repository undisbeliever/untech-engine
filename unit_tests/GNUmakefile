
.PHONY: all
all: static-tests audio-api-test


BINARY         := bin/unit-tests.sfc
MAIN           := src/unit-tests.asm

MEMORY_MAP     := hirom

PROJECT_FILE   := resources/unit-tests.utproject

TAD_PROJECT    := ../terrific-audio-driver/examples/example-project.terrificaudio

SRC_DIRS       := src resources
UNTECH_DIR     := ..

RESOURCES      := gen/font-fixed.1bpp gen/lz4-test/font-fixed.1bpp.lz4b \
                  gen/lz4-test/1kzeros.lz4b gen/lz4-test/incompressable.lz4b

STATIC_TESTS   := $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)


include ../GNUmakefile.in


.PHONY: static-tests
static-tests: $(STATIC_TESTS)

static-tests/%.success.asm: FORCE_TEST
	$(bass) -strict $@

static-tests/%.error.asm: FORCE_TEST
  ifeq ($(VANILLA_BASS), n)
	python3 static-tests/errortest.py -b $(bass) $@
  else
	$(warning 'Cannot test $@ in vanilla bass: Skipped')
  endif


.PHONY: audio-api-test
audio-api-test: bin/audio-api-bass.bin bin/audio-api-64tass.bin
	diff --report-identical-files bin/audio-api-bass.bin bin/audio-api-64tass.bin

bin/audio-api-bass.bin: audio-api-test/bass.asm $(wildcard $(UNTECH_DIR)/src/audio/*.inc)
	$(bass) -strict -o '$@' audio-api-test/bass.asm

bin/audio-api-64tass.bin: audio-api-test/64tass.asm $(wildcard ../terrific-audio-driver/audio-driver/64tass-api/*.inc)
	64tass -a -x -X --nostart -Wall -C -o '$@' audio-api-test/64tass.asm


FORCE_TEST:

ifdef BASS_DIR
  FORCE_TEST: bass REALLY_FORCE_TEST

  REALLY_FORCE_TEST:
endif


gen/font-fixed.1bpp: resources/text/font-fixed.png
	$(untech-png2tileset) -b 1 -o '$@' '$<'

gen/lz4-test/font-fixed.1bpp.lz4b: gen/font-fixed.1bpp
	$(untech-lz4c) -v -o '$@' '$<'

gen/lz4-test/%.lz4b: gen/lz4-test/%.bin
	$(untech-lz4c) -v -o '$@' '$<'

gen/lz4-test/%.bin: resources/lz4-test/%.asm
	$(bass) -strict -o '$@' '$<'

