MT_TILESETS_SRC := $(wildcard metatiles/*.aseprite)
MT_TILESETS     := $(patsubst %.aseprite,%-frame-0.png,$(MT_TILESETS_SRC))

BG_IMAGES_SRC   := $(wildcard bg-images/*.aseprite)
BG_IMAGES       := $(patsubst %.aseprite,%.png,$(BG_IMAGES_SRC))

UTSI_IMAGES_SRC := $(wildcard metasprites/*.aseprite metasprites/*/*.aseprite)
UTSI_IMAGES     := $(patsubst %.aseprite,%.png,$(UTSI_IMAGES_SRC))


.PHONY: all
all: $(MT_TILESETS) $(BG_IMAGES) $(UTSI_IMAGES)


metatiles/%-frame-0.png: metatiles/%.aseprite
	aseprite --batch '$<' --save-as '$@'

bg-images/%.png: bg-images/%.aseprite
	aseprite --batch '$<' --save-as '$@'


metasprites/%.png: metasprites/%.aseprite
	aseprite --batch '$<' --sheet-columns 6 --sheet '$@'

