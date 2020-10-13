CXX = g++
OPT = -O3
#OPT = -g
WARN = -Wall
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

# List of all .cpp files
SIM_SRC = main.cpp cache.cpp replacement.cpp helpers.cpp

# List corresponding compiled object files (.o files)
SIM_OBJ = main.o cache.o replacement.o helpers.o
 
#################################

# default rule

all: sim_cache
	@echo "my work is done here..."
	

# rule for making sim_cache

sim_cache: $(SIM_OBJ)
	$(CXX) -o sim_cache $(CFLAGS) $(SIM_OBJ) -lm
	@echo "-----------DONE WITH SIM_CACHE-----------"


# generic rule for converting any .cpp file to any .o file
.cpp.o:
	$(CXX) $(CFLAGS)  -c $*.cpp


# type "make clean" to remove all .o files plus the sim_cache binary

clean:
	rm -f *.o sim_cache


# type "make clobber" to remove all .o files (leaves sim_cache binary)

clobber:
	rm -f *.o


