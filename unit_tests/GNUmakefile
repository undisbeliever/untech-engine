
BINARY		= bin/unit-tests.sfc
MAIN		= src/unit-tests.asm

SRC_DIRS       := src ../src
INCLUDES       := $(foreach d,$(SRC_DIRS),$(wildcard $d/*.* $d/*/*.* $d/*/*/*.* $d/*/*/*/*.*))

STATIC_TESTS   := $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)

RESOURCE_DIRS   = resources/metasprite resources/text ../tables


ifeq ($(OS),Windows_NT)
  NULL_FILE = nul
else
  NULL_FILE = /dev/null
endif

.DELETE_ON_ERROR:


.PHONY: all
all: static-tests unit-tests



.PHONY: static-tests
static-tests: $(STATIC_TESTS)

# Have to use a NULL FILE to get bass's pc() function to work
static-tests/%.success.asm: FORCE
	bass-untech -strict -create -o $(NULL_FILE) $@

static-tests/%.error.asm: FORCE
	python3 static-tests/errortest.py $@

FORCE:



.PHONY: unit-tests
unit-tests: $(BINARY)

$(BINARY) $(BINARY:.sfc=.sym): $(MAIN) $(INCLUDES) test-resources bin/
	bass-untech -strict -create -o $@ -sym $(@:.sfc=.sym) $(MAIN)

bin/:
	mkdir -p $@


.PHONY: test-resources
test-resources:
	$(foreach d,$(RESOURCE_DIRS), $(MAKE) -C $d;)


.PHONY: clean
clean:
	$(RM) $(BINARY) $(BINARY:.sfc=.sym)
	$(foreach d,$(RESOURCE_DIRS), $(MAKE) -C $d clean;)

