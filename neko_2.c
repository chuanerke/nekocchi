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

#define neko_width 32
#define neko_height 32
static char neko_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x04,
    0x40, 0x10, 0x10, 0x02, 0x80, 0x28, 0x28, 0x01, 0x00, 0x49, 0x24, 0x00,
    0x06, 0x44, 0x44, 0x60, 0x18, 0x84, 0x42, 0x18, 0x60, 0x82, 0x83, 0x06,
    0x00, 0x02, 0x80, 0x00, 0x00, 0x22, 0x88, 0x00, 0x0f, 0x22, 0x88, 0x78,
    0x00, 0x22, 0x88, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x3a, 0xb9, 0x00,
    0x00, 0x04, 0x40, 0x00, 0x00, 0x08, 0x20, 0x00, 0x00, 0x70, 0x1c, 0x02,
    0x00, 0x40, 0x04, 0x05, 0x00, 0x20, 0x88, 0x04, 0x00, 0x10, 0x50, 0x02,
    0x00, 0x08, 0x20, 0x01, 0x00, 0x0b, 0xa0, 0x01, 0x80, 0x0c, 0x61, 0x02,
    0x40, 0x18, 0x31, 0x04, 0x40, 0x10, 0x11, 0x04, 0xc0, 0x11, 0x11, 0x07,
    0x60, 0x90, 0x13, 0x0c, 0xe0, 0xff, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define awake_mask_width 32
#define awake_mask_height 32
static char awake_mask_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x04,
    0x40, 0x10, 0x10, 0x02, 0x80, 0x38, 0x38, 0x01, 0x00, 0x79, 0x3c, 0x00,
    0x06, 0x7c, 0x7c, 0x60, 0x18, 0xfc, 0x7e, 0x18, 0x60, 0xfe, 0xff, 0x06,
    0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x0f, 0xfe, 0xff, 0x78,
    0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00, 0x00, 0xfe, 0xff, 0x00,
    0x00, 0xfc, 0x7f, 0x00, 0x00, 0xf8, 0x3f, 0x00, 0x00, 0xf0, 0x1f, 0x02,
    0x00, 0xc0, 0x07, 0x07, 0x00, 0xe0, 0x8f, 0x07, 0x00, 0xf0, 0xdf, 0x03,
    0x00, 0xf8, 0xff, 0x01, 0x00, 0xfb, 0xff, 0x01, 0x80, 0xff, 0xff, 0x03,
    0xc0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x07, 0xc0, 0xff, 0xff, 0x07,
    0xe0, 0xff, 0xff, 0x0f, 0xe0, 0xff, 0xfe, 0x0f, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static int w_depth;


// Window  XCreateWindow(Display  *display,  Window  parent,  int  x, int y, unsigned int width, unsigned int
//             height, unsigned int border_width, int depth, unsigned int class,  Visual  *visual,  unsigned  long
//             valuemask, XSetWindowAttributes *attributes);
Window create_win(Display *disp) {
    int screen;
    screen = DefaultScreen(disp);
    Window temp_win;
    
    w_depth = DefaultDepth(disp, screen);

    uint32_t value_mask;
    XSetWindowAttributes attr;

    // attr.border_pixel = BlackPixel(disp, screen);
    attr.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
    attr.override_redirect = True;

    

    // CWBackPixel vs CWBorderPixel ? 
    value_mask = CWBackPixel | CWEventMask | CWOverrideRedirect;
    attr.background_pixel = BlackPixel(disp, screen);

    temp_win = XCreateWindow(
        disp, RootWindow(disp, screen), 0, 0, neko_width, neko_height, 0, 
        w_depth, InputOutput, CopyFromParent, 
        value_mask, &attr
    );

    XSelectInput(disp, temp_win, attr.event_mask);

    // XMapWindow(disp, temp_win);

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
    // int value_mask = 0;
    // XGCValues values;
    // values.background = WhitePixel(disp, DefaultScreen(disp));
    // values.foreground = BlackPixel(disp, DefaultScreen(disp));
    // values.line_style = LineSolid;
    // values.cap_style = CapButt;
    // values.join_style = JoinBevel;

    // will it even check the values without mask val?
    gc = XCreateGC(disp, win, 0, NULL);
    XSetForeground(disp, gc, BlackPixel(disp, DefaultScreen(disp)));
    XSetBackground(disp, gc, WhitePixel(disp, DefaultScreen(disp)));

    return gc;
} 

Pixmap initial_draw(Display *disp, Window win) {
    int screen = DefaultScreen(disp);
    Pixmap init_neko = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), neko_bits, neko_width, neko_height, 
        BlackPixel(disp, screen), WhitePixel(disp, screen), 
        1
    );


    // IMPORTANT : IF DEPTH NOT AT 1 IT WILL NOT WORK, I WILL SEE WHY LATER
    Pixmap neko_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), awake_mask_bits, awake_mask_width, awake_mask_height, 
        WhitePixel(disp, screen), BlackPixel(disp, screen), 
        1
    );

    // printf("%d\n", init_neko);

    // XCopyPlane(disp, init_neko, win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
    // printf("%d", w_depth);
    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, neko_mask, ShapeSet);
        XMapWindow(disp, win);
    // XFlush(disp);

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
