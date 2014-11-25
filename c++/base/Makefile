CC = g++

OPT = -Wall -Wno-deprecated -g -DDEBUG -lpthread `mysql_config --libs`

SrcDir = .
SRCS = $(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:%.cpp=%.o) cJSON.o  prename.o mysql.o 

LIBBASE = libbase.a 

all: $(LIBBASE)

$(LIBBASE): $(OBJS)
	ar -r $@ $(OBJS)

%.o:%.cpp
	$(CC) $(OPT) -c $< -o $@ 
%.o:%.c
	gcc -c $< -o $@

clean:
	rm -f $(LIBBASE) *.o

