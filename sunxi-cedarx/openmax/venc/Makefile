
include ../../LIBRARY/config.mk

################################################################################
## config tool chains
################################################################################
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))

# set compile tool chains to be arm-linux-gnueabihf- set.
CC    = arm-linux-gnueabihf-gcc
CPP   = arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip
AR    = arm-linux-gnueabihf-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))

# set compile tool chains to be arm-none-linux-gnueabi- set.
CC    = arm-none-linux-gnueabi-gcc
CPP   = arm-none-linux-gnueabi-g++
STRIP = arm-none-linux-gnueabi-strip
AR    = arm-none-linux-gnueabi-ar

endif


################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = 
CONFIG_FOR_LINK = 


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ../
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/libOmxVenc.so   ## output target.

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif


################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SvnPath = $(shell find ./ -type d | grep ".svn")
SourcePath := $(filter-out $(SvnPath) $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the source files.
CppSourceFiles = $(shell find ./ -name "*.cpp")
CSourceFiles = $(shell find ./ -name "*.c");
sSourceFiles = $(shell find ./ -name "*.s");

## SourceFiles  = \
## 	./src/omx_core_cmp.cpp 	\
##     ./src/aw_omx_core.c     \
##     ./src/aw_registry_table.c             
                
  

## set the object files.
## ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))
CppObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(CppSourceFiles)))))
CObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(CSourceFiles)))))
sObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(sSourceFiles)))))


## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
					-I../omxcore/inc	\
                    -I../../LIBRARY/    \
					-I../../LIBRARY/MEMORY/ \
					-I../../LIBRARY/CODEC/VIDEO/ENCODER/ \


## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -mfpu=neon -O2 -fPIC -ldl -march=armv7-a 
#-mfloat-abi=softfp

## set link flags
LoadFlags = $(CONFIG_FOR_LINK) -ldl -lvencoder \
            -lMemAdapter -L../../LIBRARY/MEMORY  \
			-L../../LIBRARY/EXTERNAL/lib32/$(LIBDIR) -shared 


################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -f $(ObjectPath)/*
	-rm -rf $(Target)

cleanall: clean
	-rm -f $(DependFilePath)/*
	-rm -rf $(BuildPath)



################################################################################
## define target dependencies.
################################################################################

## compile source files to object files.
#$(ObjectPath)/%.o:%.cpp
#	$(CC) $(CompileFlags) -o $@ -c $<
#$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
#	$(CPP) $(CompileFlags) -o $@ -c $<



$(CppObjectFiles):$(ObjectPath)/%.o:%.cpp
	$(CPP) $(CompileFlags) -o $@ -c $<
$(CObjectFiles):$(ObjectPath)/%.o:%.c
	$(CPP) $(CompileFlags) -o $@ -c $<
$(sObjectFiles):$(ObjectPath)/%.o:%.s
	$(CPP) $(CompileFlags) -o $@ -c $<


## link object files to the target share library.
$(Target):$(CppObjectFiles) $(CObjectFiles) $(sObjectFiles)
	$(CPP) -o $@ $^ $(LoadFlags)

## set rules to generate .d files.
$(DependFilePath)/%.d:%.cpp
	set -e; rm -f $@; \
	$(CPP) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,\($*\)\.o[:]*,$(ObjectPath)/\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif


