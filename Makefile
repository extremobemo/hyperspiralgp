#
# Cubic map Raylib program
# Copyright (C) 2024
#   

TARGET = Hyper-Spiral-GP.elf
OBJS = src/main.o src/ship/ship.o src/track/track.o romdisk.o
KOS_ROMDISK_DIR = romdisk

KOS_CFLAGS += -I${KOS_PORTS}/include/raylib

all: $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean: rm-elf
	-rm -f src/*.o src/ship/*.o romdisk.o

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -lraylib -lGL -lkosutils 

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)

