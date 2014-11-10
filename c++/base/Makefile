CC = g++

OPT = -Wall -Wno-deprecated -g

SrcDir = .
SRCS = $(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:%.cpp=%.o) 

LIBBASE = libbase.a 
PRELIB = libprename.a

all: $(LIBBASE) $(PRELIB)

$(LIBBASE): $(OBJS)
	ar -r $@ $(OBJS)

$(PRELIB):prename.o
	ar -r $@ $^
	
%.o:%.cpp
	$(CC) $(OPT) -c $< -o $@ 
%.o:%.c
	gcc -c $< -o $@

clean:
	rm -f $(LIBBASE) *.o

