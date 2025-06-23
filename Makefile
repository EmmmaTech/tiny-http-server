CC := clang

BUILD := build
EXEC := $(BUILD)/httpsrv

LIBS := openssl
INCLUDES := src
SRC := $(wildcard src/**/*.c) $(wildcard src/*.c)
OBJS := $(foreach file,$(SRC),$(BUILD)/$(file:.c=.o))

CFLAGS := -Wall -Wextra
CFLAGS += $(foreach dir, $(INCLUDES), -I$(dir))
CFLAGS += $(foreach lib, $(LIBS), $(shell pkg-config --cflags $(lib)))
LFLAGS := $(foreach lib, $(LIBS), $(shell pkg-config --libs $(lib)))

.PHONY: all clean

all: $(OBJS)
	@echo "Linking $(EXEC)"
	$(CC) $(OBJS) -o $(EXEC) $(LFLAGS)

$(OBJS): | $(BUILD)

$(BUILD):
	@mkdir -p $@

$(BUILD)/%.o: %.c
	@echo "Compiling $< -> $@"
	@mkdir -p $(@D)
	$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@rm -rf $(BUILD)
