CC=g++
#CC=x86_64-w64-mingw32-g++
#DEBUG=-D_DEBUG_

#CFLAGS32=-m32 -L/usr/lib32/ -lstdc++
CFLAGS=-static $(CFLAGS32) -I./includes -I$(IDIR) -Wall -O2 -std=gnu++11 $(DEBUG)
IDIR=.

ODIR=obj
DIR_GUARD=@mkdir -p $(@D)

#LDIR =./lib

LIBS=-lm

_DEPS = makeTSX.h TZX.h TZX_Blocks.h WAV.h BlockRipper.h ByteBuffer.h includes/types.h \
		rippers/MSX4B_Ripper.h \
		rippers/B10_Standard_Ripper.h \
		rippers/B12_PureTone_Ripper.h \
		rippers/B20_Silence_Ripper.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ =	makeTSX.o ByteBuffer.o TZX.o TZX_Blocks.o WAV.o BlockRipper.o \
		MSX4B_Ripper.o \
		B10_Standard_Ripper.o \
		B12_PureTone_Ripper.o \
		B20_Silence_Ripper.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))

$(ODIR)/%.o: %.c $(DEPS)
	$(DIR_GUARD)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: %.cpp $(DEPS)
	$(DIR_GUARD)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ODIR)/%.o: rippers/%.cpp $(DEPS)
	$(DIR_GUARD)
	$(CC) -c -o $@ $< $(CFLAGS)

makeTSX: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~
