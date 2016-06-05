
UTMS_SRC	= $(wildcard utms/*.utms)
FORMATS_SRC	= $(wildcard formats/*.utft)

.DELETE_ON_ERROR:

.PHONY: all
all: metasprites.gen.inc

metasprites.gen.inc: $(UTMS_SRC) $(FORMATS_SRC)

metasprites.gen.inc: metasprites.utex
	untech-msc "$<" >| "$@"


.PHONY: clean
clean:
	$(RM) metasprites.gen.inc

