
.DELETE_ON_ERROR:

.PHONY: all
all: font-fixed.2bpp font-fixed.clr

font-fixed.2bpp font-fixed.clr: font-fixed.pcx
	pcx2snes -n -s8 -c4 -o2 font-fixed
	mv font-fixed.pic font-fixed.2bpp

.PHONY: clean
clean:
	$(RM) font-fixed.2bpp font-fixed.clr

