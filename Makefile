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
OBJS=gencp.o

prefix=/usr
SHAREDIR=$(DESTDIR)/share/cerverpages
BINDIR=$(DESTDIR)/bin
LIBDIR=$(DESTDIR)/lib
INCDIR=$(DESTDIR)/include
SUBDIRS=examples/item
LIBS=-L/usr/lib -lm -lpcre ${GLIB_LIBS} -ltemplate
OBJS=stringutil.o
WARN=-Wall
CFLAGS=-g $(DEFINES) $(WARN) $(INCLUDES) $(GLIB_CFLAGS) 
#LDFLAGS=-shared -Wl,-rpath,/usr/lib

all: gencp$(EXE)

clean:
	rm -f test.h test.c test_docgi.c test$(EXE) *.tmp core *.stackdump *.o gencp$(EXE)

gencp$(EXE): $(OBJS) main.o
	gcc -g $(LDFLAGS) $(WARN) -o $@ $^ $(LIBS)                                        
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
	./gencp -t templates $<


test.o: test.c

test: gencp${EXE}
	./gencp --verbose -t templates test.html
	${COMPILE.C} test.c
	${COMPILE.C} test_docgi.c
	gcc -g -o test test.o test_docgi.o -L/usr/lib
	./test

test_docgi.o: test_docgi.c


.PHONY: test
