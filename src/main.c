/*
 *
 */

#include "pebble.h"
#include "fctx/fctx.h"
#include "fctx/ffont.h"
#include "debug.h"

// --------------------------------------------------------------------------
// globals
// --------------------------------------------------------------------------

struct Globals {

	FPoint center;
	FPoint origin;
	int32_t rotation;

	Window* window;
	Layer* layer;
	FFont* font;
    AppTimer* timer_handle;
    uint32_t timer_timeout;

} g;

static void demo_update(void* data);
static void demo_draw(Layer* layer, GContext* ctx);

// --------------------------------------------------------------------------
// init
// --------------------------------------------------------------------------

static void init() {

    setlocale(LC_ALL, "");

    g.font = ffont_create_from_resource(RESOURCE_ID_DIN_FFONT);
    if (g.font) {
    	ffont_debug_log(g.font);
    }

	g.rotation = 0;
	g.center = FPointI(144 / 2, 168 / 2);
	g.origin = g.center;

    g.window = window_create();
    window_set_background_color(g.window, GColorBlack);
    window_stack_push(g.window, true);
    Layer* window_layer = window_get_root_layer(g.window);
    GRect window_frame = layer_get_frame(window_layer);

    g.layer = layer_create(window_frame);
    layer_set_update_proc(g.layer, &demo_draw);
    layer_add_child(window_layer, g.layer);

    g.timer_timeout = 35;
    g.timer_handle = app_timer_register(g.timer_timeout, &demo_update, NULL);
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

// --------------------------------------------------------------------------

static void demo_update(void* data) {

	g.rotation += 100;
    layer_mark_dirty(g.layer);
    g.timer_handle = app_timer_register(g.timer_timeout, &demo_update, NULL);
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

void demo_draw(Layer* layer, GContext* ctx) {

	fixed_t ri = INT_TO_FIXED(50);
	fixed_t rt = INT_TO_FIXED(52);
	fixed_t ro = INT_TO_FIXED(68);
	char text[3];
	int32_t text_rotation = 90 * TRIG_MAX_ANGLE / 360;
	int32_t text_size = 18;
	GRect bounds = layer_get_bounds(layer);

	// Draw a color test pattern for the background.
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

	// Draw a black box to highlight the text.
	fill.origin.x = FIXED_TO_INT(g.origin.x + ri) + 1;
	fill.size.w = (FIXED_TO_INT(ro) - FIXED_TO_INT(ri)) - 2;
	fill.size.h = text_size;
	fill.origin.y = FIXED_TO_INT(g.origin.y) - fill.size.h / 2;
	graphics_context_set_fill_color(ctx, COLOR_FALLBACK(GColorBlack, GColorBlack));
	graphics_fill_rect(ctx, fill, 2, GCornersAll);


	// Draw a rotating ring of numbers.
	FContext fctx;
	fctx_init_context(&fctx, ctx);
	fctx_set_fill_color(&fctx, GColorWhite);
	fctx_set_color_bias(&fctx, 0);
	fctx_begin_fill(&fctx);
	fctx.scale_from   = FPoint(FIXED_TO_INT(g.font->units_per_em), FIXED_TO_INT(-g.font->units_per_em));
	fctx.scale_to     = FPoint(text_size, text_size);
	fctx.rotation     = g.rotation;
	for (int h = 0; h < 24; ++h) {
		snprintf(text, sizeof text, "%02d", h);
		int32_t a = g.rotation + hourAngle(h);
		fctx.offset = cartesianPoint(rt, a);
		fctx.rotation = text_rotation + a;
		fctx_draw_string(&fctx, text, g.font, GTextAlignmentLeft, FTextAnchorMiddle);
	}
	fctx_plot_circle(&fctx, &g.center, ri);
	fctx_plot_circle(&fctx, &g.center, ro);
	fctx_end_fill(&fctx);

	// Draw a fixed ring of thin marks.
    FPoint points[4];
    points[0].x = points[1].x = ri;
    points[2].x = points[3].x = ri - INT_TO_FIXED(6);
    points[0].y = points[3].y = FIXED_POINT_SCALE / 2;
    points[1].y = points[2].y = -FIXED_POINT_SCALE / 2;
    fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorBlack, GColorWhite));
	fctx_set_color_bias(&fctx, 0);
    fctx_begin_fill(&fctx);
	fctx.offset = g.origin;
	for (int h = 0; h < 24; h += 1) {
		fctx.rotation = hourAngle(h);
        fctx_draw_path(&fctx, points, 4);
	}
    fctx_end_fill(&fctx);

	// Draw a rotating bezier shape.
	fctx_begin_fill(&fctx);
	fctx.offset = g.origin;
	fctx.rotation = -g.rotation;
	fctx.scale_from = FPoint(60, 60);
	fctx.scale_to = FPoint(40, 40);
	fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorBlack, GColorWhite));
	fctx_set_color_bias(&fctx, 0);
	fctx_move_to (&fctx,                                       FPointI(-20, -50));
	fctx_curve_to(&fctx, FPointI(-25, -60), FPointI( 25, -60), FPointI( 20, -50));
	fctx_curve_to(&fctx, FPointI(  0,   0), FPointI(  0,   0), FPointI( 20,  50));
	fctx_curve_to(&fctx, FPointI( 25,  60), FPointI(-25,  60), FPointI(-20,  50));
	fctx_curve_to(&fctx, FPointI(  0,   0), FPointI(  0,   0), FPointI(-20, -50));
	fctx_end_fill(&fctx);

	// Draw an orbiting pair of alpha blended circles.
	points[0] = cartesianPoint(INT_TO_FIXED(30), g.rotation + hourAngle(6));
	points[1] = cartesianPoint(INT_TO_FIXED(30), g.rotation + hourAngle(18));
	fctx_begin_fill(&fctx);
	fctx_set_fill_color(&fctx, COLOR_FALLBACK(GColorLiberty, GColorWhite));
	fctx_set_color_bias(&fctx, -2);
	fctx_plot_circle(&fctx, &points[0], INT_TO_FIXED(10));
	fctx_plot_circle(&fctx, &points[1], INT_TO_FIXED(10));
	fctx_end_fill(&fctx);

	fctx_deinit_context(&fctx);

}
