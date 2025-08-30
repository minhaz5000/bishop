CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I include

SRCS     := src/tokenizer.cpp src/nfa.cpp
OBJS     := $(patsubst %.cpp,%.o,$(SRCS))

.PHONY: all clean

all: libbishop.a

libbishop.a: $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libbishop.a
