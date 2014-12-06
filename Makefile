CC = g++
CFLAGS  = #-g -Wall
LDFLAGS = -lboost_filesystem -lboost_system
INCLUDES = -I/usr/include/boost

TARGET = xcobb

SOURCES = xcobb.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -o $(TARGET) $(SOURCES)

clean:
	$(RM) $(TARGET)

