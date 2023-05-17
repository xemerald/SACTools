#
#
#
CFLAG = /usr/bin/gcc -Wall -O3 -flto -g -I./include
SRC = ./src
#UTILITY = $(SRC)/iirfilter.o $(SRC)/picker_wu.o $(SRC)/sac_proc.o

all: scnl_mod_sac concat_sac preproc_sac integral_sac

scnl_mod_sac: $(SRC)/scnl_mod_sac.o $(SRC)/sac_proc.o
	$(CFLAG) -o $@ $(SRC)/scnl_mod_sac.o $(SRC)/sac_proc.o

concat_sac: $(SRC)/concat_sac.o $(SRC)/sac_proc.o
	$(CFLAG) -o $@ $(SRC)/concat_sac.o $(SRC)/sac_proc.o

preproc_sac: $(SRC)/preproc_sac.o $(SRC)/sac_proc.o
	$(CFLAG) -o $@ $(SRC)/preproc_sac.o $(SRC)/sac_proc.o

integral_sac: $(SRC)/integral_sac.o $(SRC)/sac_proc.o $(SRC)/iirfilter.o
	$(CFLAG) -o $@ $(SRC)/integral_sac.o $(SRC)/sac_proc.o $(SRC)/iirfilter.o -lm

# Compile rule for Object
%.o:%.c
	$(CFLAG) -c $< -o $@

# Clean-up rules
clean:
	(cd $(SRC); rm -f *.o *.obj *% *~; cd -)

clean_bin:
	rm -f $(BIN_NAME)
