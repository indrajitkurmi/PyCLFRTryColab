CXX       := g++
CXX_FLAGS := -Wall -Wextra -std=c++17 -ggdb

BIN     := bin
SRC     := src
INCLUDE := include
LIB     := lib
LIBRARIES := $(shell pkg-config --static --libs glfw3 assimp)
EXECUTABLE  := main


all: $(BIN)/$(EXECUTABLE)

run: clean all
	clear
	@echo "Executing..."
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp $(SRC)/*.c
	@echo "Building..."
	$(CXX) $(CXX_FLAGS) $(shell pkg-config --cflags glfw3 assimp) $(foreach d, $(INCLUDE), -I$d) -L$(LIB) -lpthread $^ -o $@ $(LIBRARIES)

clean:
	@echo "Clearing..."
	-rm $(BIN)/*
