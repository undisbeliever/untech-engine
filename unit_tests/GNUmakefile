
STATIC_TESTS    = $(wildcard static-tests/*.success.asm static-tests/*/*.success.asm)
STATIC_TESTS   += $(wildcard static-tests/*.error*.asm static-tests/*/*.error*.asm)

ifeq ($(OS),Windows_NT)
  NULL_FILE = nul
else
  NULL_FILE = /dev/null
endif

.PHONY: all
all: static-tests

.PHONY: static-tests
static-tests: $(STATIC_TESTS)

# Have to use a NULL FILE to get bass's pc() function to work
static-tests/%.success.asm: FORCE
	bass -strict -create -o $(NULL_FILE) $@ > $(NULL_FILE)

# Have to use a NULL FILE to get bass's pc() function to work
# ::TODO mark the error line in source and use script to verify error::
static-tests/%.error.asm: FORCE
	! bass -strict -create -o $(NULL_FILE) $@ > $(NULL_FILE)

FORCE:

