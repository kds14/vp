#include "vp.h"

void Display::ready() {
	//SDL_SetRenderTarget(renderer, texture);
}

void Display::clear() {
	SDL_RenderClear(renderer);
}

void Display::display() {
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

void Display::write(YUVData &data) {
	SDL_UpdateYUVTexture(texture, NULL, data.y_plane,
		data.y_pitch, data.u_plane, data.uv_pitch,
		data.v_plane, data.uv_pitch);
}

void Display::write(uint8_t *pixels, size_t n) {
	void *dest;
	int pitch;
	SDL_LockTexture(texture, NULL, &dest, &pitch);
	memcpy(dest, pixels, sizeof(n));
	SDL_UnlockTexture(texture);
}

bool Display::init() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
		return false;
	}
	renderer = nullptr;
	window = nullptr;
	SDL_CreateWindowAndRenderer(width * scale, height * scale, 0,
								&window, &renderer);
	if (window == NULL || renderer == NULL) {
		printf("Window/renderer creation error! SDL_Error: %s\n",
				SDL_GetError());
		return false;
	} else {
		SDL_RenderSetScale(renderer, scale, scale);
		SDL_SetWindowTitle(window, title.c_str());
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderClear(renderer);
		texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12,
				SDL_TEXTUREACCESS_STATIC, width, height);

		SDL_RenderPresent(renderer);
	}
	return true;
}

void Display::kill() {
	if (renderer)
		SDL_DestroyRenderer(renderer);
	if (window)
		SDL_DestroyWindow(window);
	SDL_Quit();
}

bool Display::wait(double duration) {
	SDL_Event event;
	if (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT)
			return false;
	}
	SDL_Delay(33);/*
	uint32_t ticks = SDL_GetTicks();
	double dur = duration * 1000;
	double diff = ticks - prev_ticks + remainder;
	double leftover = dur - diff;
	double remainder = leftover - (int)leftover;
	cout << "DUR: " << dur << " vs diff: " << diff << endl;
	if (diff < dur && leftover > 0) {
		cout << "WAIT: " << leftover << endl;
		SDL_Delay(leftover);
	} else {
		remainder = 0;
	}
	prev_ticks = SDL_GetTicks();*/
	return true;
}

Display::Display(int width, int height, int scale, string title)
	: width(width), height(height), prev_ticks(0), title(title),
	scale(scale), remainder(0)
{
	init();
}
