INCLUDES=-I/usr/include
EXE=
GLIB=glib
GLIB_LIBS=${shell pkg-config --libs $(GLIB)}

#GLIB_A=/usr/lib/libglib-2.0.dll.a
GLIB_A=/usr/lib/libglib.a
LIBS=-L/usr/lib -lm -lpcre ${GLIB_LIBS}
GLIB_CFLAGS=${shell pkg-config --cflags $(GLIB)}
WARN=-Wall
CFLAGS=-g $(WARN) 
CC=gcc
OBJS=main.o

prefix=/usr
SHAREDIR=$(DESTDIR)/share/cerverpages
BINDIR=$(DESTDIR)/bin
LIBDIR=$(DESTDIR)/lib
INCDIR=$(DESTDIR)/include
SUBDIRS=examples/item
LIBS=-L/usr/lib -lm -lpcre ${GLIB_LIBS} -ltemplate
OBJS=
WARN=-Wall
CFLAGS=-g $(DEFINES) $(WARN) $(INCLUDES) $(GLIB_CFLAGS) 
#LDFLAGS=-shared -Wl,-rpath,/usr/lib

all: gencp$(EXE)

clean:
	rm -f test.h test.c test_docgi.c test$(EX) *.tmp core *.stackdump *.o gencp$(EXE)

gencp$(EXE): gencp.o  $(OBJS)
	gcc -g $(LDFLAGS) $(WARN) -o $@ $< $(OBJS) $(LIBS)                                        
gencp.o: gencp.c
gencp$(EXE): gencp.o

install: 
	install -m755 cerverpages.py $(BINDIR)
	install -m644 cerverpages.h $(INCDIR)
	make -C templates install

uninstall:
	rm -vf $(BINDIR)/cerverpages.py
	make -C templates uninstall

examples:
	make -C examples/item

%.c: %.html
	./gencp $<

runtest:
	rm -f test.h test.c test_docgi.c
	./gencp test.html

test$(EXE): test.o test_docgi.o 
	gcc -g -o $@ test.o test_docgi.o -L/usr/lib

test_docgi.o: test_docgi.c

