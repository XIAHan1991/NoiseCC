#
INST_DIR = $(HOME)/bin
BIN  = whiten_phamp

fflags = -O2 -Wall -ffixed-line-length-none 
cflags = -O2 -Wall 

LDLIBS =   -L/PARA/pp466/Software/FFTW/lib -lfftw3

FFLAGS = $(DBG) $(fflags)

CFLAGS = $(DBG) $(cflags)

FC = gfortran

CC = gcc

DBG = -g

FOBJS = driver_c.o whiten_phamp.o swapn.o

$(BIN) : $(FOBJS)
	$(FC) $(FFLAGS) $(CFLAGS) $(FOBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(FOBJS)

