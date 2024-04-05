#
#
#
CFLAG = /usr/bin/gcc -Wall -O3 -flto -g -I./include
SRC = ./src
INSTALL_DIR = /usr/local/bin
#
PROGS = \
	sac_concat \
	sac_int \
	sac_mscnl \
	sac_preproc

all: $(PROGS)

sac_mscnl: $(SRC)/sac_mscnl.o $(SRC)/sac.o
	$(CFLAG) -o $@ $(SRC)/sac_mscnl.o $(SRC)/sac.o

sac_concat: $(SRC)/sac_concat.o $(SRC)/sac.o
	$(CFLAG) -o $@ $(SRC)/sac_concat.o $(SRC)/sac.o

sac_preproc: $(SRC)/sac_preproc.o $(SRC)/sac.o
	$(CFLAG) -o $@ $(SRC)/sac_preproc.o $(SRC)/sac.o

sac_int: $(SRC)/sac_int.o $(SRC)/sac.o $(SRC)/iirfilter.o
	$(CFLAG) -o $@ $(SRC)/sac_int.o $(SRC)/sac.o $(SRC)/iirfilter.o -lm

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
