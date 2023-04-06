EXE	=	main

MAIN 	= 	main.cc

CC	=	g++

COPTS	=	-fPIC -DLINUX -Wall 
#COPTS	=	-g -fPIC -DLINUX -Wall 

#DEBUG_LEVEL = -DDEBUG

DEBUG_LEVEL = -DNODEBUG

FLAGS	=	-Wall -s
#FLAGS	=	-Wall

DEPLIBS	=     -l ncurses -lc -lm -lpthread

GTKFLAGS = `pkg-config --cflags gtk+-2.0` 

GTKLIBS =`pkg-config --libs gtk+-2.0`

PACKAGE_LIBS =   
#-lgobject-2.0 -lgmodule-2.0 -ldl -lglib-2.0

PACKAGE_CFLAGS = -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/atk-1.0 -I/usr/include/cairo -I/usr/include/pango-1.0 -I/usr/include/freetype2 -I/usr/include/libpng12  

LIBS	=	

INCLUDEDIR =	-I.

OBJS	=      interface.o support.o callback.o crate_lib.o CamacCrate.o

INCLUDES =	

ROOT_LIBS = `root-config --glibs`
ROOT_FLAGS = `root-config --cflags`
#########################################################################

all	:	$(EXE)

clean	:
		/bin/rm -f $(OBJS) $(EXE)

$(EXE)	:	$(OBJS) $(MAIN)
		/bin/rm -f $(EXE)
		rootcint -f mydict.cxx -c MyLinkDef.h
		$(CXX) $(INCLUDEDIR) -fPIC -O3 -c mydict.cxx $(ROOT_FLAGS) -o mydict.o
		$(CXX) $(FLAGS) $(INCLUDEDIR) $(PACKAGE_LIBS) $(DEBUG_LEVEL) $(PACKAGE_CFLAGS) $(ROOT_FLAGS) $(ROOT_LIBS) -o $(EXE) $(OBJS) $(DEPLIBS) $(GTKFLAGS) $(GTKLIBS)  $(MAIN) mydict.o $(LIBS) $(ROOT_LIBS)

$(OBJS)	:	$(INCLUDES) Makefile

%.o	:	%.c
		$(CC) $(COPTS) $(PACKAGE_CFLAGS) $(GTKFLAGS) $(DEBUG_LEVEL) $(INCLUDEDIR) $(ROOT_FLAGS) -c -o $@ $< 

%.o	:	%.cc
		$(CXX) $(COPTS) $(PACKAGE_CFLAGS) $(GTKFLAGS) $(DEBUG_LEVEL) $(INCLUDEDIR) $(ROOT_FLAGS) -c -o $@ $< 
