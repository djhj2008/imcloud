C_SRC := sample.c

CFLAGS := -g -lm -O2 -Werror -Wall \
  ./libple.a

RM := rm -rf
ELF ?= $(basename $(firstword $(C_SRC)))
OBJ := $(patsubst %.c,%.o,$(C_SRC))

.PHONY: all
all: $(ELF)

.PHONY:
clean:
	$(RM) $(ELF) $(OBJ)

$(ELF): $(OBJ)
	$(CROSS_COMPILE)gcc -o $@ $(OBJ) $(CFLAGS)

$(OBJ): %.o: %.c
	$(RM) $(ELF)
	$(CROSS_COMPILE)gcc -c $^ -o $@



