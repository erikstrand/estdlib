# Makefile for Database

# compilation variables
CXX = g++
#CXXFLAGS = -Wall -o3 -std=c++11
CXXFLAGS = -Wall -g -std=c++11

# convenience variables
bindir = bin
hdir   = h
cppdir = cpp
hppdir = hpp
Includes = -I$(hdir) -I$(hppdir)

# rules
$(bindir)/main : main.cpp $(bindir)/MemoryPoolF.o $(bindir)/MemoryPool.o $(bindir)/BitField.o
	$(CXX) $(CXXFLAGS) $(Includes) -o bin/main main.cpp $(bindir)/MemoryPoolF.o $(bindir)/BitField.o

$(bindir)/MemoryPoolF.o : $(cppdir)/MemoryPoolF.cpp $(hdir)/MemoryPoolF.h $(hdir)/BitField.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(Includes)

$(bindir)/MemoryPool.o : $(cppdir)/MemoryPool.cpp $(hdir)/MemoryPool.h
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(Includes)$(bindir)/BitField.o

$(bindir)/BitField.o : $(cppdir)/BitField.cpp $(hdir)/BitField.h 
	$(CXX) $(CXXFLAGS) -c -o $@ $< $(Includes)

.PHONY : clean
clean :
	rm -v $(bindir)/*

