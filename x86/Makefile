PKG = src
SRCS = $(wildcard $(PKG)/inc/*.c) $(wildcard $(PKG)/*.c)
OBJ = $(patsubst %.c,%.o,$(SRCS))

CFLAGS := -g -m64 -O2 -Werror -Wall \
  -I./include \
  -I./plc_lib_intel \
  -lcurl \
  -lssl \
  -lcrypto \
  -ljson-c \
  -lpthread \
  ./plc_lib_intel/libple32.a

RM := rm -rf
ELF ?= imcloud
#OBJ := $(patsubst %.c,%.o,$(DIR))

.PHONY: all
all:$(ELF)

.PHONY:
clean:
	$(RM) $(ELF) $(OBJ)

$(ELF): $(OBJ)
	$(CROSS_COMPILE)gcc -o $@ $(OBJ) $(CFLAGS)

$(OBJ): %.o:%.c
	$(CROSS_COMPILE)gcc -c $^ -o $@ $(CFLAGS)



