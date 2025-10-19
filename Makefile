CXX = g++
CXXFLAGS = -Wall -std=c++20
INCLUDES = -Icodeword -Itablet -Itarabar -Imain -I/opt/homebrew/Cellar/utf8cpp/4.0.7/include
LIBS =

SRCS = main/main.cpp codeword/codeword.cpp tablet/tablet.cpp tarabar/tar.cpp main/utils.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = rgz

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)