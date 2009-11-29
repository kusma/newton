ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitPro")
endif

DEVKITARM  = $(DEVKITPRO)/devkitARM
LIBGBA     = $(DEVKITPRO)/libgba
PIMPMOBILE = ..

PREFIX ?= arm-eabi-
CC      = $(PREFIX)gcc
CXX     = $(PREFIX)g++
OBJCOPY = $(PREFIX)objcopy
STRIP   = $(PREFIX)strip
LD      = $(PREFIX)g++
AS      = $(PREFIX)as
AR      = $(PREFIX)ar

CPPFLAGS = -I$(DEVKITARM)/include -I$(LIBGBA)/include
CFLAGS   = -mthumb-interwork -mlong-calls
CXXFLAGS = -mthumb-interwork -mlong-calls -fconserve-space -fno-rtti -fno-exceptions
LDFLAGS  = -mthumb-interwork -Wl,--gc-section -Wl,-Map,$(basename $@).map -L$(LIBGBA)/lib -lgba
ASFLAGS  = -mthumb-interwork  -x assembler-with-cpp -trigraphs

ARM   = -marm
THUMB = -mthumb

ifeq ($(DEBUG), 1)
	CPPFLAGS += -DDEBUG
	CXXFLAGS += -g3 -ggdb
	CFLAGS   += -g3 -ggdb
	OBJS     += src/pimp_mixer_portable.o
else
	CPPFLAGS += -DRELEASE -DNDEBUG
	CXXFLAGS += -O3 -fomit-frame-pointer
	CFLAGS   += -O3 -fomit-frame-pointer
endif


TARGET = example.gba
OBJS   = example.o data.o adpcm_gba.o adpcm.iwram.o

all: $(TARGET) encoder

data.o : data.S test.adpcm

encoder : encoder.c
	gcc encoder.c -o encoder

$(TARGET:.gba=.elf): $(OBJS)

test_ref: test.c adpcm_ref.c
	gcc test.c adpcm_ref.c -o test_ref

test: test.c adpcm.c
	gcc test.c adpcm.c -o test
	
	
run_test: test test_ref
	./test_ref > out1.txt
	./test > out2.txt
	diff out1.txt out2.txt
	rm out1.txt out2.txt

clean:
	$(RM) $(TARGET) $(TARGET:.gba=.elf) $(TARGET:.gba=.map) $(OBJS) $(OBJS:.o=.d)

%.elf:
	$(LD) -specs=gba.specs $^ $(LDFLAGS) -o $@

%.gba: %.elf
	$(OBJCOPY) -O binary $< $@
	padbin 256 $@
	gbafix $@ -t$(basename $@)

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(THUMB) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.iwram.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(ARM) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

%.o: %.S
	$(CC) $(ASFLAGS) -c $< -o $@ -MMD -MP -MF $(@:.o=.d)

-include $(OBJS:.o=.d)
