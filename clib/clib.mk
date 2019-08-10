SRC_C += $(CLIBDIR)/dma.c
SRC_C += $(CLIBDIR)/malloc.c
SRC_C += $(CLIBDIR)/printf.c

SRC_C += $(wildcard $(CLIBDIR)/libm/*.c)
SRC_C += $(wildcard $(CLIBDIR)/libm/arm32/*.c)

INCDIRS		+= $(CLIBDIR)/include

