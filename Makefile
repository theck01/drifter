HEAP_SIZE      = 8388208
STACK_SIZE     = 61800

PRODUCT = Colony.pdx
C_DIR = C/
C_SOURCE_FILES = $(shell find $(C_DIR) -name '*.c')

# Locate the SDK
SDK = ${PLAYDATE_SDK_PATH}
ifeq ($(SDK),)
$(error SDK path not found; set ENV value PLAYDATE_SDK_PATH)
endif

# List C source files here
SRC = $(C_SOURCE_FILES)

# List all user directories here
UINCDIR = 

# List user asm files
UASRC = 

# List all user C define here, like -D_DEBUG=1
UDEFS = -Wdouble-promotion -Wall -Wextra -g

# Define ASM defines here
UADEFS = 

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

include $(SDK)/C_API/buildsupport/common.mk

sanity:
	@echo "C_DIR: $(C_DIR)"
	@echo "C_SOURCE_FILES: $(C_SOURCE_FILES)"
