#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

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

#define petal_cursor_width 9
#define petal_cursor_height 14
#define petal_cursor_x_hot 4
#define petal_cursor_y_hot 12
static unsigned char petal_cursor_bits[] = {
   0x00, 0x00, 0x6c, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00,
   0x7c, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0x38, 0x00, 0x38, 0x00, 0x10, 0x00,
   0x10, 0x00, 0x00, 0x00};

#define petal_cursor_mask_width 9
#define petal_cursor_mask_height 14
#define petal_cursor_mask_hot 0
static unsigned char petal_cursor_mask_bits[] = {
   0x6c, 0x00, 0xfe, 0x00, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01, 0xff, 0x01,
   0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x7c, 0x00, 0x7c, 0x00, 0x38, 0x00,
   0x38, 0x00, 0x10, 0x00};


#define S_WIDTH 600
#define S_HEIGHT 340
#define B_WIDTH 5

// #define WIN_NAME "Shinon"
// #define ICON_NAME "Sh"

static Display *disp;
static int screen;
static Window win;
static Window w_win, b_win;  
static GC gc;
static XSetWindowAttributes attr;
static XSetWindowAttributes w_att, b_att;
static XWindowChanges alter;
static XSizeHints wmsize;
static XWMHints wmhints;
static XTextProperty x_win_name, x_icon_name;
static uint32_t value_mask;

static Pixmap neko_map;
static Pixmap b_arr, f_arr;
static Cursor cursor;

static XColor exact, closest, front, backing;

static char *win_name = "Shinon";
static char *icon_name = "Sh";


uint32_t black, white;

void init_x() {
    disp = XOpenDisplay((char *)0);
    screen = DefaultScreen(disp);


    black = BlackPixel(disp, screen);
    white = WhitePixel(disp, screen);

    // attr.background_pixel = WhitePixel(disp, screen);
    attr.border_pixel = BlackPixel(disp, screen);
    
    XAllocNamedColor(disp, XDefaultColormap(disp, screen), "red", &exact, &closest);
    attr.background_pixel = closest.pixel;
    attr.event_mask = ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask;
    value_mask = CWBackPixel | CWBorderPixel | CWEventMask;


    // win = XCreateSimpleWindow(disp, DefaultRootWindow(disp), 0, 0, S_WIDTH, S_HEIGHT, B_WIDTH, white, black);
    win = XCreateWindow(
        disp, RootWindow(disp, screen), 200, 200, S_WIDTH, S_HEIGHT, 
        B_WIDTH, DefaultDepth(disp, screen), InputOutput, DefaultVisual(disp, screen),
        value_mask, &attr
    );

    wmsize.flags = USPosition | USSize;
    XSetWMNormalHints(disp, win, &wmsize);
    wmhints.initial_state = NormalState;
    wmhints.flags = StateHint;
    XSetWMHints(disp, win, &wmhints);
    XStringListToTextProperty(&win_name, 1, &x_win_name);
    XSetWMName(disp, win, &x_win_name);
    XStringListToTextProperty(&icon_name, 1, &x_icon_name);
    XSetWMIconName(disp, win, &x_icon_name);

    neko_map = XCreatePixmapFromBitmapData(disp, win, neko_bits, neko_width, neko_height, black, white, DefaultDepth(disp, screen));
    
    b_arr = XCreatePixmapFromBitmapData(disp, win, petal_cursor_bits, petal_cursor_width, petal_cursor_height, 1, 0, 1);
    f_arr = XCreatePixmapFromBitmapData(disp, win, petal_cursor_mask_bits, petal_cursor_mask_width, petal_cursor_mask_height, 1, 0, 1);

    // printf("b_arr = %d\n", b_arr);

    XAllocNamedColor(disp, XDefaultColormap(disp, screen), "black", &exact, &front);
    XAllocNamedColor(disp, XDefaultColormap(disp, screen), "white", &exact, &backing);

    cursor = XCreatePixmapCursor(disp, f_arr, b_arr, &front, &backing, petal_cursor_x_hot, petal_cursor_y_hot);

    XDefineCursor(disp, win, cursor);

    w_att.event_mask = ButtonPressMask | ExposureMask;
    w_att.background_pixel = WhitePixel(disp, screen);

    b_att.event_mask = ButtonPressMask | ExposureMask;
    b_att.background_pixel = BlackPixel(disp, screen);

    w_win = XCreateWindow(
        disp, win, 100, 50, 100, 100, 1, DefaultDepth(disp, screen), 
        InputOutput, DefaultVisual(disp, screen), value_mask, &w_att
    );
    
    b_win = XCreateWindow(
        disp, win, 300, 50, 50, 50, 1, DefaultDepth(disp, screen), 
        InputOutput, DefaultVisual(disp, screen), value_mask, &b_att
    );


    // gc = XCreateGC(disp, win, 0, NULL);
    // XSetForeground(disp, gc, black);
    // XSetBackground(disp, gc, white);
    // attr.background_pixel = BlackPixel(disp, screen);

    // rover = XCreateWindow(
    //     disp, win, 100, 30, 50, 70, 2, DefaultDepth(disp, screen), InputOutput,
    //     DefaultVisual(disp, screen), value_mask, &attr
    // );

    // value_mask = CWX | CWY;

    XMapWindow(disp, win);
    XMapWindow(disp, w_win);
    XMapWindow(disp, b_win);
}

void close_x() {
    XFreeGC(disp, gc);
    XDestroyWindow(disp, win);
    XDestroyWindow(disp, w_win);

    XDestroyWindow(disp, b_win);

    XCloseDisplay(disp);

    exit(1);
}

int main() {

    init_x();

    XEvent event;
    KeySym key;
    
    int x, y;
    char text[255];
    while (1) {
        XNextEvent(disp, &event);
        
        switch (event.type) {
        case Expose:
            break;
        
        // case ButtonPress:
        //     if (event.xbutton.button == Button1) {
        //         x = event.xbutton.x;
        //         y = event.xbutton.y;
        //         XCopyPlane(disp, neko_map, win, gc, 0, 0, neko_width, neko_height, x, y, 1);
        //     }
        //     break;
        
        // case KeyPress:
        //     if (XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
        //         if (text[0] == 'q') {
        //             close_x();
        //         }
        //     }
        }
        
        // if (event.type == Expose && event.xexpose.count == 0) {


        // // }
        // if (event.type == ConfigureNotify) {
        //     XMapWindow(disp, rover);
        //     sleep(1);
        //     x += 5;
        //     y += 6;
        // }
        // if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
        //     if (text[0] == 'q') {
        //         close_x();
        //     }
        //     printf("%c\n", text[0]);
        // }
    }

    return 0;
}