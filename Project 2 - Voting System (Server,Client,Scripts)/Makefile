CC := g++  # Compiler

SRCDIR := src
INCDIR := include
BUILDDIR := build
TARGET := bin/server bin/client

# Set the compiler flags
CFLAGS := -I$(INCDIR)

# Source files
SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))

.PHONY: all clean

# Default rule to build all targets
all: $(TARGET)

# Rule to build the server binary
bin/server: $(BUILDDIR)/server.o $(BUILDDIR)/poller.o
	$(CC) $(CFLAGS) $(BUILDDIR)/server.o $(BUILDDIR)/poller.o -o poller -lpthread

# Rule to build the client binary
bin/client: $(BUILDDIR)/client.o
	$(CC) $(CFLAGS) $< -o pollSwayer -lpthread

# Rule to compile server.cpp
$(BUILDDIR)/server.o: $(SRCDIR)/server.cpp $(INCDIR)/server.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lpthread

$(BUILDDIR)/poller.o: $(SRCDIR)/poller.cpp $(INCDIR)/server.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lpthread

# Rule to compile client.cpp
$(BUILDDIR)/client.o: $(SRCDIR)/client.cpp $(INCDIR)/server.hpp
	$(CC) $(CFLAGS) -c $< -o $@ -lpthread

# Clean the build directory
clean:
	rm poller pollSwayer
	rm -rf $(BUILDDIR)/*.o $(TARGET)