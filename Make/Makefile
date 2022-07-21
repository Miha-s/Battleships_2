CXX = g++
CXXFLAGS = -g -Wall
LIBS = 
FILES = server sockets
HEAD_DIR = ../Battleships/
OBJS = $(patsubst %, %.o, $(FILES))
SRC_DIR  = ../src/

##################################################

%.o: $(SRC_DIR)%.cpp $(HEAD_DIR)%.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

server: $(SRC_DIR)main.cpp $(OBJS)
	$(CXX) $(CXXFLAGS) $< $(OBJS) $(LIBS) -o $@
