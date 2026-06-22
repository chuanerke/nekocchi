#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/extensions/shape.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <error.h>
#include <string.h>
#include <unistd.h>

#define neko_width 33
#define neko_height 33
static unsigned char neko_bits[] = {
    0xff,0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff,0x01,
    0xff,0xff,0xff,0xff,0x01,0xff,0xef,0xef,0xff,0x01,0xff,0xcf,0xe7,0xff,0x01,
    0xff,0xc7,0xc7,0xff,0x01,0xff,0x87,0xc3,0xff,0x01,0xff,0x83,0x83,0xff,0x01,
    0xff,0x03,0x80,0xff,0x01,0xff,0x23,0x88,0xff,0x01,0xff,0x23,0x88,0xff,0x01,
    0xff,0x23,0x88,0xff,0x01,0xff,0x03,0x80,0xff,0x01,0xff,0x3b,0xb9,0xff,0x01,
    0xff,0x07,0xc0,0xff,0x01,0xff,0x0f,0xe0,0xff,0x01,0xff,0x7f,0xfc,0xff,0x01,
    0xff,0x7f,0xfc,0xff,0x01,0xff,0x3f,0xf8,0xff,0x01,0xff,0x1f,0xf0,0xff,0x01,
    0xff,0x0f,0xe0,0xff,0x01,0xff,0x0f,0xe0,0xff,0x01,0xff,0x0c,0x61,0xfe,0x01,
    0x7f,0x18,0x31,0xfc,0x01,0x7f,0x10,0x11,0xfc,0x01,0xff,0x11,0x11,0xff,0x01,
    0x7f,0x90,0x13,0x8c,0x01,0xff,0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff,0x01,
    0xff,0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff,0x01 
};

// Window  XCreateWindow(Display  *display,  Window  parent,  int  x, int y, unsigned int width, unsigned int
//             height, unsigned int border_width, int depth, unsigned int class,  Visual  *visual,  unsigned  long
//             valuemask, XSetWindowAttributes *attributes);
Window create_win(Display *disp) {
    int screen;
    screen = DefaultScreen(disp);
    Window temp_win;

    uint32_t value_mask;
    XSetWindowAttributes attr;

    attr.border_pixel = BlackPixel(disp, screen);
    attr.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;

    value_mask = CWBorderPixel | CWEventMask;
    temp_win = XCreateWindow(
        disp, RootWindow(disp, screen), 0, 0, neko_width, neko_height, 1, 
        DefaultDepth(disp, screen), InputOutput, DefaultVisual(disp, screen), 
        value_mask, &attr
    );

    XMapWindow(disp, temp_win);

    return temp_win;
}

void set_hints(Display *disp, Window win) {
    XWMHints *wm_hints = XAllocWMHints();
    XSizeHints *wm_size = XAllocSizeHints();

    wm_size->flags = USPosition | USSize | PMaxSize;
    wm_size->max_width = neko_width;
    wm_size->max_height = neko_height;

    wm_hints->initial_state = NormalState;
    wm_hints->flags = StateHint;

    XSetWMHints(disp, win, wm_hints);
    XSetWMNormalHints(disp, win, wm_size);

    XFree(wm_hints);
    XFree(wm_size);
}

GC create_gc(Display *disp, Window win) {
    GC gc;
    int value_mask = 0;
    XGCValues values;
    values.background = WhitePixel(disp, DefaultScreen(disp));
    values.foreground = BlackPixel(disp, DefaultScreen(disp));
    values.line_style = LineSolid;
    values.cap_style = CapButt;
    values.join_style = JoinBevel;

    // will it even check the values without mask val?
    gc = XCreateGC(disp, win, value_mask, &values);

    return gc;
} 

Pixmap initial_draw(Display *disp, Window win) {
    int screen = DefaultScreen(disp);
    Pixmap init_neko = XCreatePixmapFromBitmapData(
        disp, win, neko_bits, neko_width, neko_height, 
        BlackPixel(disp, screen), WhitePixel(disp, screen), 
        DefaultDepth(disp, screen) 
    );

    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, init_neko, ShapeSet);


    return init_neko;
}

int main() {
    Display *disp;
    Window root_win;
    int screen;
    GC gc;

    disp = XOpenDisplay((char *)0);
    screen = DefaultScreen(disp);
    root_win = create_win(disp);

    set_hints(disp, root_win);

    gc = create_gc(disp, root_win);
    Pixmap neko = initial_draw(disp, root_win);

    XEvent event;

    for ( ;; ) {
        XNextEvent(disp, &event);

        switch (event.type) {
        case Expose:
            XCopyPlane(disp, neko, root_win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
            break;
        case ButtonPress:
            if (event.xbutton.button == Button1) {
                XFreeGC(disp, gc);
                // XFreePixmap(neko);
                XDestroyWindow(disp, root_win);
                XCloseDisplay(disp);
                exit(1);
            }
            break;
        }
        
    }

    return 0;
}
