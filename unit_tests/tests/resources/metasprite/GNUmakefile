
UTSI_SRC	= $(wildcard utsi/*.utsi)
UTMS_SRC	= $(wildcard utms/*.utms)
FORMATS_SRC	= $(wildcard formats/*.utft)
IMAGES		= $(wildcard utsi/*.png)

.DELETE_ON_ERROR:

.PHONY: all
all: metasprites.gen.inc

metasprites.gen.inc: $(UTMS_SRC) $(UTSI_SRC) $(FORMATS_SRC) $(IMAGES)

metasprites.gen.inc: metasprites.utmspro
	untech-msc -o "$@" "$<"

.PHONY: clean
clean:
	$(RM) metasprites.gen.inc

