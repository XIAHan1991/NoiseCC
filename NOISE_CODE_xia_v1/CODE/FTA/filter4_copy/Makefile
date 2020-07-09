#
INST_DIR = $(HOME)/bin
BIN  = filter4

fflags = -O2 -Wall -ffixed-line-length-none
cflags = -O2 -Wall 
#fflags = -O2 -Wall -ffixed-line-length-none 
#cflags = -O2 -Wall 

LDLIBS =  -lfftw3
#LDLIBS = -L/usr/local/lib -lfftw3 -lm
#LDLIBS = -L./ -lfftw3
FFLAGS = $(DBG) $(fflags)

CFLAGS = $(DBG) $(cflags)

FC = gfortran

CC = gcc

DBG = -g

FOBJS = driver_c.o filter4.o swapn.o

$(BIN) : $(FOBJS)
	$(FC) $(FFLAGS) $(CFLAGS) $(FOBJS) -o $(BIN) $(LDLIBS) 

install :: $(BIN)
	install -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(FOBJS)
