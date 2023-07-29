# Define the symbols we might want to change:
CXX	:= g++
OBJECTS	:= src/parse.o src/main.o src/shell.o
INLUDES := includes/

main: $(OBJECTS)
	$(CXX) -ggdb3 -g $(OBJECTS) -o mysh

parsing.o : include/parsing.hpp
	$(CXX) $(CXXFLAGS) -c parse.cpp

shell.o : include/parsing.hpp include/shell.hpp
	$(CXX) -c shell.cpp

main.o: include/parsing.hpp include/shell.hpp
	$(CXX) -g $(INCLUDES) $(CXXFLAGS) -c main.cpp -o shell.o parse.o -ggdb3

clear:
	rm ./mysh
	rm src/*.o
