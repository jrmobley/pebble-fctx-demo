/*
 *
 */

#include "pebble.h"
#include "fctx/fctx.h"
#include "fctx/ffont.h"
#include "demo.h"
#include "debug.h"

// --------------------------------------------------------------------------
// Global state.
// --------------------------------------------------------------------------

State g;

// --------------------------------------------------------------------------
// init
// --------------------------------------------------------------------------

static void demo_layer_update(Layer* layer, GContext* ctx) {
    demo_draw(&g, layer, ctx);
}

static void init() {

    setlocale(LC_ALL, "");

    demo_init(&g);

    g.window = window_create();
    window_set_background_color(g.window, GColorBlack);
    window_stack_push(g.window, true);
    Layer* window_layer = window_get_root_layer(g.window);
    GRect window_frame = layer_get_frame(window_layer);

    g.layer = layer_create(window_frame);
    layer_set_update_proc(g.layer, &demo_layer_update);
    layer_add_child(window_layer, g.layer);

    g.timer_timeout = 35;
    g.timer_handle = app_timer_register(g.timer_timeout, &demo_update, &g);
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

static void deinit() {
	app_timer_cancel(g.timer_handle);
    window_destroy(g.window);
    layer_destroy(g.layer);
	ffont_destroy(g.font);
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

int main() {
    init();
    app_event_loop();
    deinit();
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void demo_init(State* state) {

    state->rotation = 0;
	state->center = FPointI(144 / 2, 168 / 2);
	state->origin = g.center;
    state->font = ffont_create_from_resource(RESOURCE_ID_DIN_FFONT);
}

// --------------------------------------------------------------------------
//
// --------------------------------------------------------------------------

void demo_update(void* data) {
    State* state = (State*)data;

	state->rotation += 100;
    layer_mark_dirty(state->layer);
    state->timer_handle = app_timer_register(state->timer_timeout, &demo_update, data);
}

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

static inline FPoint cartesianPoint(fixed_t radius, int32_t angle) {
	FPoint pt;
	int32_t c = cos_lookup(angle);
	int32_t s = sin_lookup(angle);
	pt.x = g.origin.x - s * radius / TRIG_MAX_RATIO;
	pt.y = g.origin.y + c * radius / TRIG_MAX_RATIO;
	return pt;
}

/* Convert hours to angle, measured clockwise from midnight. */
static inline int32_t hourAngle(int32_t hour) {
	return hour * TRIG_MAX_ANGLE / 24;
}

void demo_draw(State* state, Layer* layer, GContext* ctx) {

	fixed_t ri = INT_TO_FIXED(50);
	fixed_t rt = INT_TO_FIXED(52);
	fixed_t ro = INT_TO_FIXED(68);
	char text[3];
	int32_t text_rotation = 90 * TRIG_MAX_ANGLE / 360;
	int32_t text_size = 18;
	GRect bounds = layer_get_bounds(layer);

	// Use standard pebble drawing to create the background pattern.
	GRect fill = bounds;
	fill.size.w /= 2;
	fill.size.h /= 2;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorFolly, GColorBlack));
	graphics_fill_rect(ctx, fill, 6, GCornerTopLeft);
	fill.origin.x = fill.size.w;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorBrightGreen, GColorBlack));
	graphics_fill_rect(ctx, fill, 6, GCornerTopRight);
	fill.origin.y = fill.size.h;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorPictonBlue, GColorBlack));
	graphics_fill_rect(ctx, fill, 6, GCornerBottomRight);
	fill.origin.x = 0;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorIcterine, GColorBlack));
	graphics_fill_rect(ctx, fill, 6, GCornerBottomLeft);
	fill.origin.x = FIXED_TO_INT(state->origin.x + ri) + 1;
	fill.size.w = (FIXED_TO_INT(ro) - FIXED_TO_INT(ri)) - 2;
	fill.size.h = text_size;
	fill.origin.y = FIXED_TO_INT(state->origin.y) - fill.size.h / 2;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorBlack, GColorBlack));
	graphics_fill_rect(ctx, fill, 2, GCornersAll);


	// Here we are going to use font rendering and circle plotting to create
    // a rotating ring of numbers.  The rendering algorithm uses and even-odd
    // fill rule, so by plotting two concentric circles, we get a filled ring.
    // And by drawing text within that ring, we effectively make a 'cutout' of
    // the text so the background shows through.
    // Also, it is important to know that the fctx_draw_* functions will use
    // the transform state of the FContext (i.e. offset, scale, and rotation).
    // The fctx_plot_* functions do not use the transform state and will plot
    // exactly the coordinates that are passed in.
	FContext fctx;
	fctx_init_context(&fctx, ctx);
	fctx_set_fill_color(&fctx, GColorWhite);
	fctx_set_color_bias(&fctx, 0);
	fctx_begin_fill(&fctx);
	fctx_set_text_size(&fctx, state->font, text_size);
	for (int h = 0; h < 24; ++h) {
		snprintf(text, sizeof text, "%02d", h);
		int32_t a = state->rotation + hourAngle(h);
        fctx_set_rotation(&fctx, text_rotation + a);
		fctx_set_offset(&fctx, cartesianPoint(rt, a));
		fctx_draw_string(&fctx, text, state->font, GTextAlignmentLeft, FTextAnchorMiddle);
	}
	fctx_plot_circle(&fctx, &state->center, ri);
	fctx_plot_circle(&fctx, &state->center, ro);
	fctx_end_fill(&fctx);

	// Here we are going to draw some very thin clock pips.  We are going to
    // use an array of path points the fctx_draw_path function to draw a rectangle
    // with different rotation transforms.
    FPoint points[4];
    points[0].x = points[1].x = ri;
    points[2].x = points[3].x = ri - INT_TO_FIXED(6);
    points[0].y = points[3].y = FIXED_POINT_SCALE / 2;
    points[1].y = points[2].y = -FIXED_POINT_SCALE / 2;
    fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorBlack, GColorWhite));
	fctx_set_color_bias(&fctx, 0);
    fctx_begin_fill(&fctx);
	fctx_set_offset(&fctx, state->origin);
	for (int h = 0; h < 24; h += 1) {
		fctx_set_rotation(&fctx, hourAngle(h));
        fctx_draw_path(&fctx, points, 4);
	}
    fctx_end_fill(&fctx);

	// Here we are going to draw a rotating vector shape.  This time we will
    // use the SVG like drawing commands to build the shape, again using the
    // transform state of the FContext for rotation, scaling and offset.
	fctx_begin_fill(&fctx);
	fctx_set_offset(&fctx, state->origin);
	fctx_set_rotation(&fctx, -state->rotation);
	fctx_set_scale(&fctx, FPoint(60, 60), FPoint(40, 40));
	fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorBlack, GColorWhite));
	fctx_set_color_bias(&fctx, 0);
	fctx_move_to (&fctx,                                       FPointI(-20, -50));
	fctx_curve_to(&fctx, FPointI(-25, -60), FPointI( 25, -60), FPointI( 20, -50));
	fctx_curve_to(&fctx, FPointI(  0,   0), FPointI(  0,   0), FPointI( 20,  50));
	fctx_curve_to(&fctx, FPointI( 25,  60), FPointI(-25,  60), FPointI(-20,  50));
	fctx_curve_to(&fctx, FPointI(  0,   0), FPointI(  0,   0), FPointI(-20, -50));
	fctx_end_fill(&fctx);

	// Finally, we will plot an orbiting pair of alpha blended circles.
    // The color bias setting is something like an alpha value.  It will modify
    // the calculated coverage amount in the anti-aliasing code, which in turn
    // will change the color blending.
	points[0] = cartesianPoint(INT_TO_FIXED(30), state->rotation + hourAngle(6));
	points[1] = cartesianPoint(INT_TO_FIXED(30), state->rotation + hourAngle(18));
	fctx_begin_fill(&fctx);
	fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorLiberty, GColorWhite));
	fctx_set_color_bias(&fctx, -2);
	fctx_plot_circle(&fctx, &points[0], INT_TO_FIXED(10));
	fctx_plot_circle(&fctx, &points[1], INT_TO_FIXED(10));
	fctx_end_fill(&fctx);

	fctx_deinit_context(&fctx);

}
