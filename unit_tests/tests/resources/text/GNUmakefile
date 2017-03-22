
.DELETE_ON_ERROR:

.PHONY: all
all: font-fixed.2bpp font-fixed.clr

font-fixed.2bpp font-fixed.clr: font-fixed.png
	untech-png2tileset -b 2 -o font-fixed.2bpp -p font-fixed.clr font-fixed.png

.PHONY: clean
clean:
	$(RM) font-fixed.2bpp font-fixed.clr

