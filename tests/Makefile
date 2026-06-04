# Makefile for CycleFlash unit tests
#
# 编译:  mingw32-make
# 运行:  mingw32-make run
# 清理:  mingw32-make clean

CC       = gcc
CFLAGS   = -std=c11 -g -O0 -Wno-int-to-pointer-cast -Wno-unused-parameter
DEFINES  = -DCFS_TEST -DCFS_DEBUG=0u -DCFS_FLASH_READ_MODE=1u
INCLUDES = -I../cycleflash

SRCS     = test_main.c mock_port.c
TARGET   = test_cycleflash.exe
DEPS     = mock_port.h ../cycleflash/cycle_flash_system.h ../cycleflash/cfs_port.h ../cycleflash/cycle_flash_system.c

all: $(TARGET)

$(TARGET): $(SRCS) $(DEPS)
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -o $@ $(SRCS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
