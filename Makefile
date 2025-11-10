CXX = g++
CXXFLAGS = -Wall -std=c++20
INCLUDES = -Icodeword -Itablet -Itarabar -Imain
LIBS = -ldl

SRCS = main/main.cpp main/menuUtils.cpp codeword/codeword.cpp tablet/tablet.cpp tarabar/tar.cpp main/utils.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rgz

# Shared libs with renamed names for dlopen
SHARED_FLAGS = -fPIC -shared
LIBCODEWORD_SO = lib/libCodewordCipher.so
LIBTABLET_SO = lib/libTabletCipher.so
LIBTARABAR_SO = lib/libTarabarCipher.so
LIBS_ALL = $(LIBCODEWORD_SO) $(LIBTABLET_SO) $(LIBTARABAR_SO)

all: $(TARGET) libs

$(TARGET): $(OBJS)
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

libs: $(LIBS_ALL)

$(LIBCODEWORD_SO): codeword/codeword.cpp main/utils.cpp
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) $^ -o $@ -ldl

$(LIBTABLET_SO): tablet/tablet.cpp main/utils.cpp
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) $^ -o $@ -ldl

$(LIBTARABAR_SO): tarabar/tar.cpp main/utils.cpp
	@mkdir -p lib
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SHARED_FLAGS) $^ -o $@ -ldl

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) $(LIBS_ALL)
	rm -rf lib

.PHONY: all libs clean