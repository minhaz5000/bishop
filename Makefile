CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2

SRCS     := tokenizer.cpp nfa.cpp
OBJS     := $(SRCS:.cpp=.o)

.PHONY: all clean

all: libbishop.a

libbishop.a: $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libbishop.a
