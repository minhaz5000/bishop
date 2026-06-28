CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2 -I include

LIB_SRCS := src/tokenizer.cpp src/nfa.cpp src/utils.cpp src/dfa.cpp src/debug.cpp src/serializer.cpp src/unicode.cpp
LIB_OBJS := $(patsubst %.cpp,%.o,$(LIB_SRCS))

TEST_SRCS := tests/test_main.cpp tests/test_tokenizer.cpp \
	tests/test_nfa.cpp tests/test_errors.cpp \
	tests/test_dfa.cpp \
	tests/test_groups.cpp \
	tests/test_flags.cpp \
	tests/test_posix.cpp \
	tests/test_boundaries.cpp \
	tests/test_lazy.cpp \
	tests/test_lookahead.cpp \
	tests/test_debug.cpp \
	tests/test_serial.cpp \
	tests/test_unicode.cpp \
	tests/test_match_iterator.cpp \
	tests/test_exhaustive.cpp

TEST_OBJS := $(patsubst %.cpp,%.o,$(TEST_SRCS))

.PHONY: all clean test bench

all: libbishop.a

libbishop.a: $(LIB_OBJS)
	ar rcs $@ $^

bishop_tests: libbishop.a $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_OBJS) -L. -lbishop

test: bishop_tests
	./bishop_tests

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(LIB_OBJS) $(TEST_OBJS) libbishop.a bishop_tests
