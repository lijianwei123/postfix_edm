#server makefile
CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g  

SrcDir = .
SRCS = $(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:%.cpp=%.o) ../base/libbase.a

SERVER = server

all: $(SERVER)

$(SERVER): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) -lpthread 

%.o:%.cpp
	$(CC) $(OPT) -I $(INC) -c $< -o $@ 

clean:
	rm -f $(SERVER) *.o
