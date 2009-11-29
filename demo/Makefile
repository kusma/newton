# Makefile for a random Shitfaced Clowns demo

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro)
endif

#GBAEMU    ?= $(DEVKITPRO)/gbaemu/vba.exe
GBAEMU    ?= $(DEVKITPRO)/gbaemu/NO\$$GBA.EXE
DEVKITARM  = $(DEVKITPRO)/devkitARM
LIBGBA     = $(DEVKITPRO)/libgba
SFCCON	   = tools/sfccon

PREFIX = arm-eabi-
CC        = $(PREFIX)gcc
CXX       = $(PREFIX)g++
OBJCOPY   = $(PREFIX)objcopy
STRIP     = $(PREFIX)strip
LD        = $(PREFIX)g++
AS        = $(PREFIX)gcc
GBAFIX    = gbafix
GFX2GBA   = ./tools/gfx2gba
CONVERTER = ./tools/converter

CPPFLAGS = -I$(DEVKITARM)/include -I$(LIBGBA)/include -Iinclude
CFLAGS   = -mthumb-interwork -mlong-calls
CXXFLAGS = -mthumb-interwork -mlong-calls -fconserve-space -fno-rtti -fno-exceptions
LDFLAGS  = -mthumb-interwork -Wl,--gc-section -Wl,-Map,$(TARGET).map
ASFLAGS  = -mthumb-interwork

ifeq ($(DEBUG), 1)
	BUILDDIR  = Debug
	CPPFLAGS += -DDEBUG
	CXXFLAGS += -O3 -fomit-frame-pointer -fno-threadsafe-statics
	CFLAGS   += -O3 -fomit-frame-pointer
	LDFLAGS  += -s
else
	BUILDDIR  = Release
	CPPFLAGS += -DRELEASE -DNDEBUG
	CXXFLAGS += -O3 -fomit-frame-pointer -fno-threadsafe-statics
	CFLAGS   += -O3 -fomit-frame-pointer
	LDFLAGS  += -s
endif

LIBS = $(LIBGBA)/lib/libgba.a

ARM   = -marm
THUMB = -mthumb

TARGET = sfc-mc
OBJS   = main.o gba_assert.o math.o fb.o palette.o part.o
OBJS  += matrix4x4.o
OBJS  += tilepool.o

# adpcm player
OBJS += adpcm.iwram.o adpcm_gba.o
OBJS += timer.o

# datafile
OBJS += data.o

# effects
OBJS += fjalloc.o decomp_arm.iwram.o flikk.o

OBJS += 3dthen.o 3dthen_arm.iwram.o poly_subpixel_arm.iwram.o antialias_arm.iwram.o
OBJS += glow.o glow_arm.iwram.o

OBJS += grid_arm.iwram.o grideffects_arm.iwram.o grideffects.o
OBJS += infzoom_asm.o
OBJS += plant.o
OBJS += noise.o

# parts
OBJS += parts/message.o
OBJS += parts/skate.o
OBJS += parts/mercury.o
OBJS += parts/growy.o
OBJS += parts/mirror.o
OBJS += parts/telos.o
OBJS += parts/cubegrid.o
OBJS += parts/lattice.o
OBJS += parts/lockedloaded.o

OBJS += parts/sphere.o
OBJS += parts/radial.o
OBJS += parts/parallax_scroll.o
OBJS += parts/zoomer.o
OBJS += parts/poelse.o
OBJS += parts/spheregrid.o
OBJS += parts/spheregrid_arm.iwram.o

DEPSDIR = $(BUILDDIR)
OFILES  = $(foreach file,$(OBJS),$(BUILDDIR)/$(notdir $(file)))
DEPENDS = $(foreach file,$(OFILES:.o=.d),$(DEPSDIR)/$(notdir $(file)))

VPATH = source source/parts data datasrc

include rules.mak

.PHONY: all clean run

all: $(BUILDDIR) $(TARGET).gba

$(BUILDDIR):
	mkdir -p $@

$(BUILDDIR)/$(TARGET).elf: $(BUILDDIR) $(OFILES) $(LIBGBA)/lib/libgba.a

$(BUILDDIR)/data.o: data/data.S data/* data/tune.adpcm data/tune2.adpcm

clean:
	$(RM) $(OFILES) $(DEPENDS) $(TARGET).gba $(BUILDDIR)/$(TARGET).elf $(TARGET).map data/scenes.S $(SCENES)

run: $(TARGET).gba
	$(GBAEMU) $(TARGET).gba

include data_rules.mak


# data dependencies
data.d: data/data.S
	@echo $(DEPSDIR)/data.o : data/data.S \\> $@
	@grep "^datafile " data/data.S | sed "s/datafile [a-zA-Z_0-9]*[,]\?[ ]*//" | xargs echo >> $@

-include $(DEPENDS)
-include data.d