CC        := clang
SRC_DIR   := src
BUILD_DIR := build
LDFLAGS   := -pthread
INCLUDES  := 
CFLAGS    = -g -Wall -O3
BINARY    := nyanko
EXT       := c

SOURCES := $(shell find $(SRC_DIR) -name '*.$(EXT)' | sort -k 1nr | cut -f2-)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.$(EXT)=$(BUILD_DIR)/%.o)
DEPS    := $(OBJECTS:.o=.d)

$(BINARY) : $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.$(EXT)
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) && mkdir $(BUILD_DIR)

-include $(DEPS)