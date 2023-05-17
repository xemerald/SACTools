#
#
#
CFLAG = /usr/bin/gcc -Wall -O3 -flto -g -I./include
SRC = ./src
INSTALL_DIR = /usr/local/bin
#
PROGS = \
	scnl_mod_sac \
	concat_sac \
	preproc_sac \
	integral_sac

all: $(PROGS)

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

#
install:
	@echo Installing to $(INSTALL_DIR)...
	@for x in $(PROGS) ; \
	do \
		cp ./$$x $(INSTALL_DIR); \
	done
	@echo Finish installing of all programs!

# Clean-up rules
clean:
	(cd $(SRC); rm -f *.o *.obj *% *~; cd -)

clean_bin:
	rm -f $(BIN_NAME)
