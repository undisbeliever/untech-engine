
TABLES = $(patsubst %.py,%.gen.inc, $(wildcard *.py))

.DELETE_ON_ERROR:

.PHONY: all
all: $(TABLES)

%.gen.inc: %.py
	python3 $< >| $@


.PHONY: clean
clean::
	$(RM) $(TABLES)

