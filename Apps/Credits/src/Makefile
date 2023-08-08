
# compile c programs with GLCC
CC=../../../Compilers/glcc/build/glcc

# use 32k map modified by overlay horizon.ovl
CFLAGS=-map=32k -rom=v6

# avoid runtime bss initialization to save code.
LDFLAGS=--no-runtime-bss-init

# target
TARGET=../Credits.gt1


$(TARGET): Credits.c scroll.s Makefile
	${CC} ${CFLAGS} ${LDFLAGS} Credits.c scroll.s -o $@

clean:
	-rm $(TARGET)

