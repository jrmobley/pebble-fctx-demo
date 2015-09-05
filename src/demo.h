
#pragma once

typedef struct State {

	FPoint center;
	FPoint origin;
	int32_t rotation;

	Window* window;
	Layer* layer;
	FFont* font;
	AppTimer* timer_handle;
	uint32_t timer_timeout;

} State;

void demo_init(State* state);
void demo_update(void* data);
void demo_draw(State* state, Layer* layer, GContext* ctx);
