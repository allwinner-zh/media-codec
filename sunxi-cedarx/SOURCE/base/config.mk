
################################################################################
## configurations.
################################################################################

## option for momory driver config.
OPTION_MEMORY_DRIVER_SUNXIMEM = 1
OPTION_MEMORY_DRIVER_ION      = 2
OPTION_MEMORY_DRIVER_VE = 4

## option for dram interface.
OPTION_DRAM_INTERFACE_DDR1_16BITS = 1
OPTION_DRAM_INTERFACE_DDR1_32BITS = 2
OPTION_DRAM_INTERFACE_DDR2_16BITS = 3
OPTION_DRAM_INTERFACE_DDR2_32BITS = 4
OPTION_DRAM_INTERFACE_DDR3_16BITS = 5
OPTION_DRAM_INTERFACE_DDR3_32BITS = 6
OPTION_DRAM_INTERFACE_DDR3_64BITS = 7

## configuration.
CC = $(OPTION_CC_GNUEABIHF)
CONFIG_OS = $(OPTION_OS_LINUX)
CONFIG_MEMORY_DRIVER = $(OPTION_MEMORY_DRIVER_VE)
CONFIG_CHIP = $(OPTION_CHIP_1651)

