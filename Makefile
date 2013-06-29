# Common macros
CXX := g++
CXXFLAGS := -Wall -std=c++0x -I./src
LIBS := -lboost_regex-mt -pthread

# Macros for build
CXXFLAGS_RELEASE := -O3 -march=native -flto -DNDEBUG
CXXFLAGS_DEBUG := -g -O0
CXX_FILES := $(shell find src \( -name \*.cpp -or -name \*.cc \) -print)
OBJS := $(shell echo $(CXX_FILES) | perl -p -e 's/.(cpp|cc)/.o/g')
TARGET := main

# Macros for test
CXXFLAGS_TEST := $(CXXFLAGS_DEBUG) -I./test
CXX_FILES_TEST := $(filter-out src/main.cpp, $(CXX_FILES)) $(shell find test \( -name \*.cpp -or -name \*.cc \) -print)
OBJS_TEST := $(shell echo $(CXX_FILES_TEST) | perl -p -e 's/.(cpp|cc)/.o/g')
TARGET_TEST := test/test

# "make release" or just "make" means release build
.PHONY: release
release: CXXFLAGS += $(CXXFLAGS_RELEASE)
release: all

# "make debug" means debug build
.PHONY: debug
debug: CXXFLAGS += $(CXXFLAGS_DEBUG)
debug: all

.PHONY: all
all: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# "make test" means test
.PHONY: test
test: CXXFLAGS += $(CXXFLAGS_TEST)
test: $(OBJS_TEST)
	$(CXX) $(CXXFLAGS) -o $(TARGET_TEST) $(OBJS_TEST) $(LIBS)
	./$(TARGET_TEST)

.PHONY: clean
clean:
	rm -f $(OBJS) $(OBJS_TEST) $(TARGET) $(TARGET_TEST)

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

.cc.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@
