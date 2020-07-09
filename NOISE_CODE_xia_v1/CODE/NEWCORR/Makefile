#
INST_DIR = ../../bin

BIN  = justCOR


fflags = -O2 -ffixed-line-length-none  -g
#fflags = -fast -Mextend -Msave
#fflags =  -O -e
cflags = -O2 -Wall -g
#cflags =  
# if you don't use shared libraries comment the next line
#shlib = -R/opt/SUNWspro/lib

FFLAGS = $(DBG)  $(fflags) $(shlib)

CFLAGS = $(DBG)  $(cflags)

LDLIBS = -L/PARA/pp466/Software/FFTW/lib -lfftw3
#LDLIBS =  -lfftw3

FC = gfortran

DBG = 

FOBJS = dcommon.o dmultifft.o justCOR.o

$(BIN) : $(FOBJS)
	$(FC) $(FFLAGS) $(CFLAGS) $(FOBJS) -o $(BIN) $(LDLIBS)

install :: $(BIN)
	install  -s $(BIN) $(INST_DIR)

clean ::
	rm -f $(BIN) core $(FOBJS)
