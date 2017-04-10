
TEST_SRC	= $(wildcard src/*.asm)
TEST_BINARIES	= $(patsubst src/%.asm,bin/%.sfc, $(TEST_SRC))

INC_DIR         = ../src/
INCLUDES	= $(wildcard $(INC_DIR)/*.* $(INC_DIR)/*/*.* $(INC_DIR)/*/*/*.*)
INCLUDES       += $(wildcard tests/*.* tests/*/*.* tests/*/*/*.*)

STATIC_TESTS    = $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)

RESOURCE_DIRS   = resources/metasprite resources/text ../tables


ifeq ($(OS),Windows_NT)
  NULL_FILE = nul
else
  NULL_FILE = /dev/null
endif

.DELETE_ON_ERROR:


.PHONY: all
all: static-tests tests



.PHONY: static-tests
static-tests: $(STATIC_TESTS)

# Have to use a NULL FILE to get bass's pc() function to work
static-tests/%.success.asm: FORCE
	bass-untech -strict -create -o $(NULL_FILE) $@

static-tests/%.error.asm: FORCE
	python3 static-tests/errortest.py $@

FORCE:



.PHONY: tests
tests: $(TEST_BINARIES)

bin/%.sfc bin/%.sym: src/%.asm bin/ $(INCLUDES) test-resources
	bass-untech -strict -create -o $@ -sym $(@:.sfc=.sym) $<

bin/:
	mkdir -p $@


.PHONY: test-resources
test-resources:
	$(foreach d,$(RESOURCE_DIRS), $(MAKE) -C $d;)


.PHONY: clean
clean:
	$(RM) $(TEST_BINARIES) $(TEST_BINARIES:.sfc=.sym)
	$(foreach d,$(RESOURCE_DIRS), $(MAKE) -C $d clean;)

