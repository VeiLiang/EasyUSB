#this dir is ARCHDIR

SRC_C += $(ARCHDIR)/lib/eabi.c

SRC_S += $(ARCHDIR)/lib/memcmp.S
SRC_S += $(ARCHDIR)/lib/memcpy.S
SRC_S += $(ARCHDIR)/lib/memmove.S
SRC_S += $(ARCHDIR)/lib/memset.S
SRC_S += $(ARCHDIR)/lib/setjmp.S
SRC_S += $(ARCHDIR)/lib/memset.S
SRC_S += $(ARCHDIR)/lib/strcmp.S
SRC_S += $(ARCHDIR)/lib/strncmp.S

INCDIRS += $(ARCHDIR)/include
