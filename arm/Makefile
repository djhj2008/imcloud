PKG = src
SRCS = $(wildcard $(PKG)/inc/*.c) $(wildcard $(PKG)/*.c)
OBJ = $(patsubst %.c,%.o,$(SRCS))

CFLAGS := -g -lm -O2 -Werror -Wall \
  -I./include \
  -I../plc_lib_intel \
  -I../libcurl/include \
  -I../libssl-dev/include \
  -I../libjson/include \
  -I../libhiredis/include \
  -lm \
  -lcurl \
  -lssl \
  -lcrypto \
  -ljson-c \
  -lpthread \
  -lple \
  -lhiredis \
  -L../libcurl/lib \
  -L../libssl-dev/lib \
  -L../libjson/lib \
  -L../plc_lib_intel \
  -L../libhiredis/lib \
  -DVERSION_MAJOR="${VERSION_MAJOR}" \
  -DVERSION_MINOR="${VERSION_MINOR}" \
  -DVERSION_REVISION="${VERSION_REVISION}" \
  -DVERSION_HOST="${VERSION_HOST}" \
  -DBUILD_DATE="${BUILD_DATE}"

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



