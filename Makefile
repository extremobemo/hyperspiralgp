#
# Cubic map Raylib program
# Copyright (C) 2024
#   

TARGET = Hyper-Spiral-GP.elf
OBJS = main.o romdisk.o
KOS_ROMDISK_DIR = romdisk skybox.vs skybox.fs

KOS_CFLAGS += -I${KOS_PORTS}/include/raylib -I/home/extremobemo/Desktop/playground/spiraldrive/r-zero/include

all: $(TARGET)

include $(KOS_BASE)/Makefile.rules

clean: rm-elf
	-rm -f $(OBJS)

rm-elf:
	-rm -f $(TARGET) romdisk.*

$(TARGET): $(OBJS)
	kos-cc -o $(TARGET) $(OBJS) -lraylib -lGL -lkosutils 

run: $(TARGET)
	$(KOS_LOADER) $(TARGET)

dist: $(TARGET)
	-rm -f $(OBJS) romdisk.img
	$(KOS_STRIP) $(TARGET)

