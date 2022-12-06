# Compiler
CC = avr-gcc
# Flags
CFLAGS = -Os -DF_CPU=16000000UL -Wl,-u,vfprintf -lprintf_flt -lm
# Final binary
BIN = $(shell basename `pwd`)


# Targeted microprocessor
INSTALL_TGT = atmega2560
# Install location
INSTALL_DIR = /dev/ttyACM0
# Install baud rate
INSTALL_BAUD = 115200
# avrdude flags
INSTALL_FLAGS = -F -V -D -c wiring 


# Build directory
BUILD_DIR = ./bin
# List of source files
SRC = $(wildcard src/*.c)
# All .o files to be built go to build directory
OBJ = $(SRC:%.c=$(BUILD_DIR)/%.o)
# All .d dependency files auto generated by gcc
DEP = $(OBJ:%.o=%.d)

# Documentation directory
DOCS_DIR = ./docs


# AVR object copy and default build target
$(BIN) : $(BUILD_DIR)/$(BIN)
	avr-objcopy -O ihex -R .eeprom $(BUILD_DIR)/$(BIN) $(BIN)

# Actual build from .o files
$(BUILD_DIR)/$(BIN) : $(OBJ)
# Make needed directories
	mkdir -p $(@D)
# Link all .o files
	$(CC) -mmcu=$(INSTALL_TGT) $(CFLAGS) $^ -o $@


# Include dependencies
-include $(DEP)


# Build for each object file
# Header dependencies are covered by
# -include $(DEP)
$(BUILD_DIR)/%.o : %.c
# Make needed directories
	mkdir -p $(@D)
# Compile to .o
# -MMD creates .d files with
# the same name as the .h file
	$(CC) -mmcu=$(INSTALL_TGT) $(CFLAGS) $(MACROS) -MMD -c $< -o $@

# Install with avrdude
install : $(BIN)
	sudo avrdude $(INSTALL_FLAGS) -p $(INSTALL_TGT) -P $(INSTALL_DIR) -b $(INSTALL_BAUD) -U flash:w:$(BIN)

# Removes all generated files
.PHONY : clean docs compile_commands
clean: 
	rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP) $(BIN) $(BIN).pdf

docs:
	cd $(DOCS_DIR);pdflatex $(BIN).tex

compile_commands:
	bear -- make
