CC ?= cc
CFLAGS := -std=c11 -O2 -Wall -Wextra -Werror
ARM_CC ?= arm-none-eabi-gcc
ARM_AR ?= ar
ARMFLAGS := -std=c11 -Os -mcpu=cortex-m4 -mthumb -ffreestanding -Wall -Wextra -Werror
SOURCES = core/morning_summary.c firmware/morning_entry.c $(filter-out ui/example.c ui/test%.c,$(wildcard ui/*.c))
OBJECTS = $(patsubst %.c,build/%.o,$(SOURCES))

.PHONY: test arm clean
test:
	mkdir -p build
	$(CC) $(CFLAGS) core/morning_summary.c core/test_morning.c -o build/test_core
	./build/test_core
	$(CC) $(CFLAGS) core/morning_summary.c ui/morning_adapter.c firmware/morning_entry.c firmware/test_entry.c -o build/test_entry
	./build/test_entry
arm: build/libmorning.a
build/libmorning.a: $(OBJECTS)
	$(ARM_AR) rcs $@ $^
build/%.o: %.c $(wildcard core/*.h firmware/*.h ui/*.h)
	mkdir -p $(@D)
	$(ARM_CC) $(ARMFLAGS) -c $< -o $@
clean:
	rm -rf build
