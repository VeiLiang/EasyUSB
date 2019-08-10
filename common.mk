# config common parameter in here

#-------Cross compile------
CROSS_COMPILE = /opt/gcc-linaro-5.3.1-2016.05-x86_64_arm-eabi/bin/arm-eabi-
CC	          = $(CROSS_COMPILE)gcc
AS	          = $(CROSS_COMPILE)gcc -x assembler-with-cpp
LD	          = $(CROSS_COMPILE)ld
OBJCOPY	      = $(CROSS_COMPILE)objcopy
OBJDUMP       = $(CROSS_COMPILE)objdump
SIZE 		  = $(CROSS_COMPILE)size

#-------common command------
RM = rm
ECHO = @echo
CP = cp
MKDIR = mkdir
SED = sed

#-------common flag------
ASFLAGS		:= -g -ggdb -Wall -O3 
CFLAGS	        := -g -ggdb -Wall -O3
CXXFLAGS	:= -g -ggdb -Wall -O3


#-------common define------

