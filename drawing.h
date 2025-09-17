#include <stdint.h>
#include <X11/Xlib.h>

typedef enum{
	NONE,
	DOT,
	LINE_TO
} action_type;

typedef struct{
	uint32_t color;
	uint16_t thickness;
	uint16_t x;
	uint16_t y;
	action_type type;
} action_point;

// note that none of these functions will flush any requests that they make


/*
 * creates GC and sets window background color, want this done here so GC is not owned by main
 */
void init_drawing(Display *disp, Window win, uint32_t background_color, uint32_t foreground_color);

/*
 * draws a dot centered at (x, y)
 */
void draw_dot(int x, int y);

/*
 * marks a point along a line and draws a line to it from the previous point used for any draw
 * returns false if called without any prior history to use
 */
bool draw_line_to(int x, int y);

/* 
 * undoes the last action, one action is any of:
 * 1. A single dot
 * 2. A series of line to draws and whatever the first line to connected with
 */
void undo_action();

/* 
 * sets drawing color to argb, where the channels correspond to bytes in the name
 */
void set_color(uint32_t argb);

/* 
 * add delta to the draw thickness
 */
void change_thickness(int delta);

/* 
 * set the draw thickness
 */
void set_thickness(int thickness);

/* 
 * cleans up dynamically allocated memory
 */
void destroy_draw();
