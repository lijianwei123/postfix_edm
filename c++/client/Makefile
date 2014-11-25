#client makefile
CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -DDEBUG -g -lrt  -lpthread `mysql_config --libs`

SrcDir = .
SRCS = $(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:%.cpp=%.o) ../base/libbase.a ../base/json/libjson.a

CLIENT = client

all: $(CLIENT)

$(CLIENT): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS)

%.o:%.cpp
	$(CC) $(OPT) -I $(INC) -c $< -o $@ 

clean:
	rm -f $(CLIENT) *.o
