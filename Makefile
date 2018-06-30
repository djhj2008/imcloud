C_SRC := imcloud.c thpool.c wifi_status.c im_file.c

CFLAGS := -g -lm -O2 -Werror -Wall \
  -I./libcurl/include \
  -I./libssl-dev/include \
  -I./libjson/include \
  -lcurl \
  -lssl \
  -lcrypto \
  -ljson-c \
  -lpthread \
  -L./libcurl/lib \
  -L./libssl-dev/lib \
  -L./libjson/lib

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



