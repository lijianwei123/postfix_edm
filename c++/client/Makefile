#client makefile
CC = g++

INC = ../base
OPT = -Wall -Wno-deprecated -g  -lpthread -lmysqlclient

SrcDir = .
SRCS = $(foreach dir,$(SrcDir),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:%.cpp=%.o) ../base/libbase.a

CLIENT = client

all: $(CLIENT)

$(CLIENT): $(OBJS)
	$(CC)  $(OPT) -o $@ $(OBJS) -lpthread 

%.o:%.cpp
	$(CC) $(OPT) -I $(INC) -c $< -o $@ 

clean:
	rm -f $(CLIENT) *.o
