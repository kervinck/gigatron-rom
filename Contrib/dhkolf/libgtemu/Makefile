CC= gcc
CFLAGS= -Wall -Os
GTLIB_O= gtsdl.o gtemu.o gtloader.o
LUADIR= ../lua-5.3.4/src

all: gtrun gtemu.so

gtrun: gtmain.o $(GTLIB_O)
	gcc -o $@ gtmain.o $(GTLIB_O) -lSDL2

gtemu.so: luagt.o $(GTLIB_O)
	$(CC) $(CFLAGS) -o $@ -shared -fPIC luagt.o $(GTLIB_O) -lSDL2

gtsdl.o: gtsdl.c gtsdl.h gtemu.h
	$(CC) $(CFLAGS) -o $@ -c $<

gtemu.o: gtemu.c gtemu.h
	$(CC) $(CFLAGS) -o $@ -c $<

gtloader.o: gtloader.c gtemu.h
	$(CC) $(CFLAGS) -o $@ -c $<

gtmain.o: gtmain.c gtemu.h gtsdl.h
	$(CC) $(CFLAGS) -o $@  -c $<

luagt.o: luagt.c gtemu.h gtsdl.h
	$(CC) $(CFLAGS) -o $@ -I $(LUADIR) -c $<

