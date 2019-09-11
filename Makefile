TARGET=./build/vp
DIR=./build/
CC=g++
CFLAGS=-std=c++17 -g
SRC=./src/*.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p $(DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(shell sdl2-config --cflags --libs) \
	-L/opt/ffmpeg -I/opt/ffmpeg/include -lavcodec -lavformat \
	#-lavfilter -lavdevice -lswresample -lswscale -lavutil

clean:
	rm -r $(DIR)
