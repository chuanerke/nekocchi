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

#define NEKO_DEPTH 1

static int w_depth;

Window create_win(Display *disp) {
    int screen;
    screen = DefaultScreen(disp);
    Window temp_win;
    
    w_depth = DefaultDepth(disp, screen);

    uint32_t value_mask;
    XSetWindowAttributes attr;

    attr.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask;
    attr.override_redirect = True;

    value_mask = CWBackPixel | CWEventMask | CWOverrideRedirect;
    attr.background_pixel = BlackPixel(disp, screen);

    unsigned int win_x, win_y;
    win_x = (DisplayWidth(disp, screen) - (neko_width / 2)) / 2;
    win_y = (DisplayHeight(disp, screen) - (neko_height / 2)) / 2;

    temp_win = XCreateWindow(
        disp, RootWindow(disp, screen), win_x, win_y, neko_width, neko_height, 0, 
        w_depth, InputOutput, CopyFromParent, 
        value_mask, &attr
    );

    XSelectInput(disp, temp_win, attr.event_mask);
    
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
        NEKO_DEPTH
    );

    Pixmap neko_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), awake_mask_bits, awake_mask_width, awake_mask_height, 
        WhitePixel(disp, screen), BlackPixel(disp, screen), 
        NEKO_DEPTH
    );

    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, neko_mask, ShapeSet);
    XMapWindow(disp, win);

    return init_neko;
}


void move_neko(Display *disp, Window win, int x, int y) {
    int err = XMoveWindow(disp, win, x, y); 
}


// Bool  XQueryPointer(Display *display, Window w, Window *root_return, Window *child_return, int *root_x_re‐
//     turn, int *root_y_return, int *win_x_return, int *win_y_return, unsigned int *mask_return);

    //  Status XGetGeometry(Display *display, Drawable d, Window *root_return, int *x_return, int  *y_return,  un‐
    //         signed  int *width_return, unsigned int *height_return, unsigned int *border_width_return, unsigned
    //         int *depth_return);



void neko_move(Display *disp, Window win) {
    Window root_ret, child_ret;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_ret;

    _Bool xq_ret = XQueryPointer(
        disp, win, &root_ret, &child_ret, &root_x, 
        &root_y, &win_x, &win_y, &mask_ret
    );
    if (!xq_ret) {
        fprintf(stderr, "XQueryPointer: Pointer not on same screen\n");
        return;
    }

    int neko_x, neko_y;
    unsigned int width_ret, height_ret, border_width_ret, dept_ret;

    XGetGeometry(disp, win, &root_ret, &neko_x, &neko_y, &width_ret, &height_ret, &border_width_ret, &dept_ret);


    printf("X: %d, Y: %d\n", root_x, root_y);
    printf("NX: %d, NY: %d\n", neko_x, neko_y);

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
    int x = 0;
    int y = 0;


    // XCopyPlane(disp, neko, root_win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
    int c = 0;
    for ( ;; ) {
        neko_move(disp, root_win);
        // XFree(win_name);

        // move_neko(disp, root_win, ++x, ++y);
        // sleep(1);
        while (XPending(disp)) {
            XNextEvent(disp, &event);
            switch (event.type) {
            case Expose:
                XCopyPlane(disp, neko, root_win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
                break;
            case ButtonPress:
                if (event.xbutton.button == Button1) {
                    XFreeGC(disp, gc);
                    XDestroyWindow(disp, root_win);
                    XCloseDisplay(disp);
                    exit(1);
                }
                break;
            }
        }
        
    }

    return 0;
}
