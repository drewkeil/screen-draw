#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>

#include "drawing.h"

int main(){
	Display *disp = XOpenDisplay(NULL);

	int screen = DefaultScreen(disp);
	int width = DisplayWidth(disp, screen);
	int height = DisplayHeight(disp, screen);

	XVisualInfo visinfo;
	XMatchVisualInfo(disp, DefaultScreen(disp), 32, TrueColor, &visinfo);

	XSetWindowAttributes attr;
	attr.colormap = XCreateColormap(disp, DefaultRootWindow(disp), visinfo.visual, AllocNone);
	attr.border_pixel = 0;
	attr.background_pixel = 0;

	Window win = XCreateWindow(disp, DefaultRootWindow(disp), 0, 0, width, height, 0, visinfo.depth, InputOutput, visinfo.visual, CWColormap | CWBorderPixel | CWBackPixel, &attr);

	XSelectInput(disp, win, StructureNotifyMask | KeyPressMask | Button1MotionMask | ButtonPressMask);

	// set window to be on top (hopefully, I have no idea if this actually works)
	// I got this from a stack overflow post TODO paste the link
	Atom state_above = XInternAtom(disp, "_NET_WM_STATE_ABOVE", 1);
	Atom net_state = XInternAtom(disp, "_NET_WM_STATE", 1);

	XClientMessageEvent xclient;
	memset(&xclient, 0, sizeof(xclient));

	xclient.type = ClientMessage;
	xclient.window = win;
	xclient.message_type = net_state;
	xclient.format = 32;
	xclient.data.l[0] = 1; // wm state add
	xclient.data.l[1] = state_above;

	XSendEvent(disp, DefaultRootWindow(disp), false, SubstructureRedirectMask | SubstructureNotifyMask, (XEvent*)&xclient);

	XFlush(disp);

	XMapWindow(disp, win);

	init_drawing(disp, win, 0, 0xffff0000);
	XClearWindow(disp, win);

	// wait for map notify
	while(true){
		XEvent e;
		XNextEvent(disp, &e);
		if(e.type == MapNotify)
			break;
	}

	uint32_t colors[10] = {0xffff0000, 0xff00ff00, 0xff0000ff, 0xff000000, 0xffffffff};

	// main loop
	bool quit = false;
	while(!quit){
		XEvent e;
		XNextEvent(disp, &e);
		if(e.type == KeyPress){
			int keysym = XLookupKeysym(&e.xkey, 0);
			switch(keysym){
				case XK_q: 
					quit = true;
					break;
				case XK_u:
					undo_action();
					break;
				case XK_1:
					set_color(colors[0]);
					break;
				case XK_2:
					set_color(colors[1]);
					break;
				case XK_3:
					set_color(colors[2]);
					break;
				case XK_4:
					set_color(colors[3]);
					break;
				case XK_5:
					set_color(colors[4]);
					break;
				default:
					break;
			}
		}else if(e.type == ButtonPress){
			switch(e.xbutton.button){
				case 1:
					draw_dot(e.xbutton.x, e.xbutton.y);
					break;
				case 4:
					change_thickness(-1);
					break;
				case 5:
					change_thickness(1);
					break;
				default: 
					break;
			}
		}else if(e.type == MotionNotify){
			// don't need to test button number because it's only listening for button 1 pressed move events
			draw_line_to(e.xmotion.x, e.xmotion.y);
		}
	}

	destroy_draw();
}
