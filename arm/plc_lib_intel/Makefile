C_SRC := sample.c

CFLAGS := -g -lm -O2 -Werror -Wall

RM := rm -rf
#ELF ?= $(basename $(firstword $(C_SRC)))
LIB :=libple.so
OBJ := libple.a

.PHONY: all
all: $(LIB)

.PHONY:
clean:
	$(RM) $(LIB)

$(LIB): $(OBJ)
	$(CROSS_COMPILE)gcc -shared -fpic -o $@ $(OBJ) $(CFLAGS)



