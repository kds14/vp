#ifndef VP_H
#define VP_H

#include <vector>
#include <string>
#include <iostream>

#include "SDL.h"

using std::vector;
using std::string;
using std::cout;
using std::endl;

class YUVData {
public:
	int y_pitch, uv_pitch;
	uint8_t *y_plane, *u_plane, *v_plane;
	int total_size;
	YUVData(int total_size, int pitch) :
			y_plane((uint8_t*)malloc(total_size)),
			total_size(total_size),
			u_plane((uint8_t*)malloc(total_size / 4)),
			v_plane((uint8_t*)malloc(total_size / 4)),
			y_pitch(pitch), uv_pitch(pitch / 2) { };

	~YUVData() {
		delete y_plane;
		delete u_plane;
		delete v_plane;
	}

	uint64_t checksum() {
		uint64_t val = -1;
		for (int i = 0; i < total_size / 8; i++) {
			val ^= ((uint64_t*)y_plane)[i];
		}
		for (int i = 0; i < total_size / 32; i++) {
			val ^= ((uint64_t*)u_plane)[i];
		}
		for (int i = 0; i < total_size / 32; i++) {
			val ^= ((uint64_t*)v_plane)[i];
		}
		return val;
	}
	void print() {
		cout << &y_plane << ", " << &u_plane << ", " << &v_plane
		<< ", " << y_pitch << ", " << uv_pitch << ", " << checksum()
		<< endl;
	}
};

class Display {
public:
	void kill();
	bool wait(double duration);
	void display();
	void ready();
	void clear();
	void write(YUVData &data);
	void write(uint8_t* pixels, size_t pixels_size);
	Display(int width, int height, int scale, string title);
private:
	bool init();
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	int width;
	int height;
	int scale;
	double fps;
	double time_per_frame;
	uint32_t prev_ticks;
	double remainder;
	string title;
};

#endif
