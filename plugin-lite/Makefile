TARGET = Tekken6Jin8Gate
OBJS = src/main.o exports.o
CFLAGS = -Os -G0 -std=c11 -Wall -Wextra -Werror
ASFLAGS = $(CFLAGS)
PSPSDK := $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
