CC = gcc
CFLAGS = -O3 -Wall
DEBUGFLAGS = -O3 -Wall -g -DDEBUG
TARGET = numbaplota
BUILD_PATH = ./
INSTALL_PATH = /usr/local/bin/
SOURCE = numbaplota.c

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(BUILD_PATH)$(SOURCE) $(CFLAGS) -o $(TARGET)

debug:
	$(CC) $(BUILD_PATH)$(SOURCE) $(DEBUGFLAGS) -o $(TARGET)

install:
	mv $(TARGET) $(INSTALL_PATH)

clean:
	$(RM) $(TARGET)
	$(RM) $(INSTALL_PATH)$(TARGET)	
