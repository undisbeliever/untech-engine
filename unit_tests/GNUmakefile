
BINARY		= bin/unit-tests.sfc
MAIN		= src/unit-tests.asm

SRC_DIRS       := src ../src resources
INCLUDES       := $(foreach d,$(SRC_DIRS),$(wildcard $d/*.inc $d/*/*.inc $d/*/*/*.inc $d/*/*/*/*.inc))

STATIC_TESTS   := $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)


.DELETE_ON_ERROR:


.PHONY: all
all: static-tests directories unit-tests



.PHONY: static-tests
static-tests: $(STATIC_TESTS)

static-tests/%.success.asm: FORCE
	bass-untech -strict $@

static-tests/%.error.asm: FORCE
	python3 static-tests/errortest.py $@

FORCE:



.PHONY: tables
TABLE_SCRIPTS  := $(wildcard ../tables/*.py)
TABLE_INCS     := $(patsubst ../tables/%.py,gen/tables/%.inc,$(TABLE_SCRIPTS))
tables: $(TABLE_INCS)

gen/tables/%.inc: ../tables/%.py
	python3 $< >| $@



.PHONY: resources
resources: metasprites font
RESOURCES :=


.PHONY: metasprites
metasprites: gen/metasprites.inc
gen/metasprites.inc: $(wildcard resources/metasprite/*.*)
gen/metasprites.inc: $(wildcard resources/metasprite/*/*.*)
gen/metasprites.inc: $(wildcard resources/metasprite/*/*/*.*)
gen/metasprites.inc: resources/metasprite/metasprites.utmspro
	untech-msc -o "$@" "$<"

RESOURCES += gen/metasprites.inc


.PHONY: font
font: gen/font-fixed.1bpp
gen/font-fixed.1bpp: resources/text/font-fixed.png
	untech-png2tileset -b 1 -o "$@" "$<"

RESOURCES += gen/font-fixed.1bpp




.PHONY: directories
DIRS := $(sort $(dir $(BINARY) $(RESOURCES) $(TABLE_INCS)))
directories: $(DIRS)
$(DIRS):
	mkdir -p $@



.PHONY: unit-tests
unit-tests: directories $(BINARY)

$(BINARY) $(BINARY:.sfc=.sym): $(MAIN) $(INCLUDES) $(TABLE_INCS) $(RESOURCES)
	bass-untech -strict -o $@ -sym $(@:.sfc=.sym) $(MAIN)


.PHONY: memory-usage
memory-usage: directories $(BINARY)
	bass-untech -d SHOW_MEMORY_USAGE -strict $(MAIN)



.PHONY: clean
clean:
	$(RM) $(BINARY) $(BINARY:.sfc=.sym)
	$(RM) $(sort $(TABLE_INCS))
	$(RM) $(sort $(RESOURCES))

