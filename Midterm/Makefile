# Compiler
CXX = g++
CXXFLAGS = -Wall -std=c++11

# Executables
TARGETS = Front_end Back_end

# Source files
FRONT_SRC = Front_end.cpp
BACK_SRC = Back_end.cpp

# Object files
FRONT_OBJ = $(FRONT_SRC:.cpp=.o)
BACK_OBJ = $(BACK_SRC:.cpp=.o)

# Default target
all: $(TARGETS)

# Build the Front_end executable
Front_end: $(FRONT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the Back_end executable
Back_end: $(BACK_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up object files and executables
clean:
	rm -f $(FRONT_OBJ) $(BACK_OBJ) $(TARGETS)

# Dependencies for object files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Phony targets
.PHONY: all clean
