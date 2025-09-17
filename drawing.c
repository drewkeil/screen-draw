#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "drawing.h"

static Display *disp;
static Window win;
static GC gc;
static XGCValues gc_attr;
static Pixmap display_buffer;

static action_point *undo_buffer = NULL;
static uint32_t buffer_index = 0; // index of first empty spot in buffer
static int buffer_capacity = 0;

static uint32_t color;
static uint32_t bg_color;

static void push_buffer(action_point *act){
	if(buffer_index == buffer_capacity){
		action_point *new_buffer = malloc(sizeof(action_point) * 2 * buffer_capacity);
		for(int i = 0; i < buffer_index; ++i)
			new_buffer[i] = undo_buffer[i];
		free(undo_buffer);
		undo_buffer = new_buffer;
		buffer_capacity *= 2;
	}

	undo_buffer[buffer_index++] = *act;
}

static void pop_buffer(action_point *act){
	if(buffer_index == 0){
		act->type = NONE;
		return;
	}

	*act = undo_buffer[--buffer_index];
}

void init_drawing(Display *display, Window window, uint32_t background_color, uint32_t foreground_color){
	undo_buffer = malloc(sizeof(action_point) * 1024);
	buffer_capacity = 1024;

	disp = display;
	win = window;

	int screen = DefaultScreen(disp);
	int width = DisplayWidth(disp, screen);
	int height = DisplayHeight(disp, screen);
	display_buffer = XCreatePixmap(disp, win, width, height, 32);

	gc_attr.cap_style = CapRound;
	gc_attr.line_width = 0;
	gc = XCreateGC(disp, win, GCCapStyle, &gc_attr);

	bg_color = background_color;

	XSetForeground(disp, gc, foreground_color);
	color = foreground_color;
}

void draw_dot(int x, int y){
	XDrawLine(disp, win, gc, x, y, x, y);

	action_point act = (action_point){color, (uint16_t)gc_attr.line_width, (uint16_t)x, (uint16_t)y, DOT};
	push_buffer(&act);
}

bool draw_line_to(int x, int y){
	if(buffer_index == 0)
		return false;

	int top_x = undo_buffer[buffer_index - 1].x;
	int top_y = undo_buffer[buffer_index - 1].y;
	XDrawLine(disp, win, gc, top_x, top_y, x, y);

	action_point act = (action_point){color, (uint16_t)gc_attr.line_width, (uint16_t)x, (uint16_t)y, LINE_TO};
	push_buffer(&act);
	return true;
}

void undo_action(){
	if(buffer_index == 0)
		return;

	action_point act;
	pop_buffer(&act);
	while(act.type == LINE_TO)
		pop_buffer(&act);

	if(buffer_index == 0){
		XClearWindow(disp, win);
		return;
	}

	int screen = DefaultScreen(disp);
	int width = DisplayWidth(disp, screen);
	int height = DisplayHeight(disp, screen);
	XSetForeground(disp, gc, bg_color);
	XFillRectangle(disp, display_buffer, gc, 0, 0, width, height);

	int prev_thickness = gc_attr.line_width;

	int prev_x = 0, prev_y = 0;
	for(int i = 0; i < buffer_index; ++i){
		set_thickness(undo_buffer[i].thickness);
		XSetForeground(disp, gc, undo_buffer[i].color);
		int x = undo_buffer[i].x;
		int y = undo_buffer[i].y;
		switch(undo_buffer[i].type){
			case DOT:
				XDrawLine(disp, display_buffer, gc, x, y, x, y);
				break;
			case LINE_TO:
				XDrawLine(disp, display_buffer, gc, prev_x, prev_y, x, y);
				break;
			default:
				break;
		}
		prev_x = undo_buffer[i].x;
		prev_y = undo_buffer[i].y;
	}
	set_thickness(prev_thickness);

	XCopyArea(disp, display_buffer, win, gc, 0, 0, width, height, 0, 0);
	XSetForeground(disp, gc, color);
}

void set_color(uint32_t argb){
	color = argb;
	XSetForeground(disp, gc, argb);
}

void change_thickness(int delta){
	gc_attr.line_width += delta;
	if(gc_attr.line_width < 0)
		gc_attr.line_width = 0;
	XChangeGC(disp, gc, GCLineWidth, &gc_attr);
}

void set_thickness(int thickness){
	gc_attr.line_width = thickness;
	XChangeGC(disp, gc, GCLineWidth, &gc_attr);
}

void destroy_draw(){
	free(undo_buffer);
	undo_buffer = NULL;
}
