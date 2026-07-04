#include "neko.h"
#include <signal.h>
#include <poll.h>

#define neko_width 32
#define neko_height 32

#define neko_speed 13
#define SECOND 1000 

static int w_depth;
static uint32_t move_value_mask = CWX | CWY;

static int neko_time = 125;



struct neko_diff {
    int rel_x;
    int rel_y;
};


Pixmap awake, down1, down2, dtogi1, dtogi2;
Pixmap dwleft1, dwleft2, dwright1, dwright2;
Pixmap jare2, kaki1, kaki2, left1, left2;
Pixmap ltogi1, ltogi2, mati2, mati3;
Pixmap right1, right2, rtogi1, rtogi2;
Pixmap sleep1, sleep2, up1, up2;
Pixmap upleft1, upleft2, upright1, upright2;
Pixmap utogi1, utogi2;

Pixmap awake_mask, down1_mask, down2_mask, dtogi1_mask, dtogi2_mask;
Pixmap dwleft1_mask, dwleft2_mask, dwright1_mask, dwright2_mask;
Pixmap jare2_mask, kaki1_mask, kaki2_mask, left1_mask, left2_mask;
Pixmap ltogi1_mask, ltogi2_mask, mati2_mask, mati3_mask;
Pixmap right1_mask, right2_mask, rtogi1_mask, rtogi2_mask;
Pixmap sleep1_mask, sleep2_mask, up1_mask, up2_mask;
Pixmap upleft1_mask, upleft2_mask, upright1_mask, upright2_mask;
Pixmap utogi1_mask, utogi2_mask;
Pixmap space_mask;

//　I don't think there's a need for multiple GCs for each bitmap
// If I add the different charas then they'll have different properties but for now
// I think one GC is fine.


struct anim_map {
    Pixmap *xmp;
    Pixmap *mask;
    char *xmp_bits;
    char *mask_bits;
};

struct anim_map maps[] =  {
    {&awake, &awake_mask, awake_bits, awake_mask_bits}, {&down1, &down1_mask, down1_bits, down1_mask_bits}, 
    {&down2, &down2_mask, down2_bits, down2_mask_bits}, {&dtogi1, &dtogi1_mask, dtogi1_bits, dtogi1_mask_bits}, 
    {&dtogi2, &dtogi2_mask, dtogi2_bits, dtogi2_mask_bits}, {&dwleft1, &dwleft1_mask, dwleft1_bits, dwleft1_mask_bits},
    {&dwleft2, &dwleft2_mask, dwleft2_bits, dwleft2_mask_bits}, {&dwright1, &dwright1_mask, dwright1_bits, dwright1_mask_bits}, 
    {&dwright2, &dwright2_mask, dwright2_bits, dwright2_mask_bits}, {&jare2, &jare2_mask, jare2_bits, jare2_mask_bits}, 
    {&kaki1, &kaki1_mask, kaki1_bits, kaki1_mask_bits}, {&kaki2, &kaki2_mask, kaki2_bits, kaki2_mask_bits}, 
    {&left1, &left1_mask, left1_bits, left1_mask_bits}, {&left2, &left2_mask, left2_bits, left2_mask_bits}, 
    {&ltogi1, &ltogi1_mask, ltogi1_bits, ltogi1_mask_bits}, {&ltogi2, &ltogi2_mask, ltogi2_bits, ltogi2_mask_bits}, 
    {&mati2, &mati2_mask, mati2_bits, mati2_mask_bits}, {&mati3, &mati3_mask, mati3_bits, mati3_mask_bits}, 
    {&right1, &right1_mask, right1_bits, right1_mask_bits}, {&right2, &right2_mask, right2_bits, right2_mask_bits}, 
    {&rtogi1, &rtogi1_mask, rtogi1_bits, rtogi1_mask_bits}, {&rtogi2, &rtogi2_mask, rtogi2_bits, rtogi2_mask_bits}, 
    {&sleep1, &sleep1_mask, sleep1_bits, sleep1_mask_bits}, {&sleep2, &sleep2_mask, sleep2_bits, sleep2_mask_bits}, 
    {&up1, &up1_mask, up1_bits, up1_mask_bits}, {&up2, &up2_mask, up2_bits, up2_mask_bits}, 
    {&upleft1, &upleft1_mask, upleft1_bits, upleft1_mask_bits}, {&upleft2, &upleft2_mask, upleft2_bits, upleft2_mask_bits}, 
    {&upright2, &upright2_mask, upright2_bits, upright2_mask_bits}, {&utogi1, &utogi1_mask, utogi1_bits, utogi1_mask_bits}, 
    {&utogi2, &utogi2_mask, utogi2_bits, utogi2_mask_bits}
};

typedef enum {
    IDLE,
    AWAKE,
    DOWN,
    DTOGI,
    DW_LEFT,
    DW_RIGHT,
    JARE,
    SLEEP,
    KAKI,
    LEFT,
    LTOGI,
    RIGHT,
    UP,
    UPLEFT,
    UPRIGHT,
    UTOGI
} STATE;


static STATE neko_state = AWAKE;
static _Bool anim_start = False;
static _Bool neko_still = True;

struct animation {
    Pixmap *xmp;
    Pixmap *mask;
};

struct animation move_states[][2] = {
    {{&mati2, &mati2_mask}, {&mati2, &mati2_mask}}, 
    {{&awake, &awake_mask}, {&awake, &awake_mask}},
    {{&down1, &down1_mask}, {&down2, &down2_mask}},
    {{&dtogi1, &dtogi1_mask}, {&dtogi2, &dtogi2_mask}},
    {{&dwleft1, &dwleft1_mask}, {&dwleft2, &dwleft2_mask}},
    {{&dwright1, &dwright1_mask}, {&dwright2, &dwright2_mask}},
    {{&jare2, &jare2_mask}, {&mati2, &mati2_mask}},
    {{&sleep1, &sleep1_mask}, {&sleep2, &sleep2_mask}},
    {{&kaki1, &kaki1_mask}, {&kaki2, &kaki2_mask}},
    {{&left1, &left1_mask}, {&left2, &left2_mask}},
    {{&ltogi1, &ltogi1_mask}, {&ltogi2, &ltogi2_mask}},
    {{&right1, &right1_mask}, {&right2, &right2_mask}},
    {{&up1, &up1_mask}, {&up2, &up2_mask}},
    {{&upleft1, &upleft1_mask}, {&upleft2, &upleft2_mask}},
    {{&upright1, &upright1_mask}, {&upright2, &upright2_mask}},
    {{&utogi1, &utogi1_mask}, {&utogi2, &utogi2_mask}}
};


void init_anim_map(Display *disp) {
    int screen = DefaultScreen(disp);

    size_t len = sizeof(maps) / sizeof(maps[0]);
    for (size_t i = 0; i < len; i++) {
        *(maps[i].xmp) = XCreatePixmapFromBitmapData(
            disp, RootWindow(disp, screen), maps[i].xmp_bits, neko_width, neko_height, 
            BlackPixel(disp, screen), WhitePixel(disp, screen), 
            NEKO_DEPTH
        );
    
        *(maps[i].mask) = XCreatePixmapFromBitmapData(
            disp, RootWindow(disp, screen), maps[i].mask_bits, neko_width, neko_height, 
            WhitePixel(disp, screen), BlackPixel(disp, screen), 
            NEKO_DEPTH
        );
    }
}

Window create_win(Display *disp) {
    int screen;
    screen = DefaultScreen(disp);
    Window temp_win;

    w_depth = DefaultDepth(disp, screen);

    uint32_t value_mask;
    XSetWindowAttributes attr;

    attr.event_mask = ExposureMask | ButtonPressMask | StructureNotifyMask |  VisibilityChangeMask;
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
        disp, RootWindow(disp, screen), awake_bits, awake_width, awake_height, 
        BlackPixel(disp, screen), WhitePixel(disp, screen), 
        NEKO_DEPTH
    );

    Pixmap neko_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), awake_mask_bits, awake_mask_width, awake_mask_height, 
        WhitePixel(disp, screen), BlackPixel(disp, screen), 
        NEKO_DEPTH
    );

    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, neko_mask, ShapeSet);

    return init_neko;
}

void poll_neko(Display *disp) {
    struct pollfd ufd;
    ufd.fd = -1;
    ufd.events = POLLIN;

    poll(&ufd, 1, neko_time);
}

void neko_animate(Display *disp, Window win, GC gc) {
    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, *(move_states[neko_state][(int)anim_start].mask), ShapeSet);
    XCopyPlane(disp, *(move_states[neko_state][(int)anim_start].xmp), win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
    XFlush(disp);

    anim_start = !anim_start;
    poll_neko(disp);
}

// void neko_redraw(Display *disp, Window win) {
//     XMapWindow(disp, win);
// }

void sleep_idle_init(Display *disp, Window win) {
    int screen = DefaultScreen(disp);
    sleep1 = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), sleep1_bits, sleep1_width, sleep1_height, 
        BlackPixel(disp, screen), WhitePixel(disp, screen), 
        NEKO_DEPTH
    );

    sleep1_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), sleep1_mask_bits, sleep1_mask_width, sleep1_mask_height, 
        WhitePixel(disp, screen), BlackPixel(disp, screen), 
        NEKO_DEPTH
    );

    sleep2 = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), sleep2_bits, sleep2_width, sleep2_height, 
        BlackPixel(disp, screen), WhitePixel(disp, screen), 
        NEKO_DEPTH
    );
    
    sleep2_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), sleep2_mask_bits, sleep2_mask_width, sleep2_mask_height, 
        WhitePixel(disp, screen), BlackPixel(disp, screen), 
        NEKO_DEPTH
    );
}

void sleep_idle_anim(Display *disp, Window win, GC gc, _Bool which) {
    if (!which) {
        XShapeCombineMask(disp, win, ShapeBounding, 0, 0, sleep1_mask, ShapeSet);
        // XMapWindow(disp, win);
        XCopyPlane(disp, sleep1, win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
    }
    else {
        XShapeCombineMask(disp, win, ShapeBounding, 0, 0, sleep2_mask, ShapeSet);
        XCopyPlane(disp, sleep2, win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
        // XMapWindow(disp, win);
    }

}

// void neko_move(Display *disp, Window win) {
    // Window root_ret, child_ret;
    // int root_x, root_y, win_x, win_y;
    // unsigned int mask_ret;

    // _Bool xq_ret = XQueryPointer(
    //     disp, win, &root_ret, &child_ret, &root_x, 
    //     &root_y, &win_x, &win_y, &mask_ret
    // );
    // if (!xq_ret) {
    //     fprintf(stderr, "XQueryPointer: Pointer not on same screen\n");
    //     return;
    // }

    // int neko_x, neko_y;
    // unsigned int width_ret, height_ret, border_width_ret, dept_ret;

    // XGetGeometry(disp, win, &root_ret, &neko_x, &neko_y, &width_ret, &height_ret, &border_width_ret, &dept_ret);

//     double angle = atan2(win_y, win_x);
//     double deg_angle = angle / M_PI * 180;

//     double x_mov = cosl(angle);
//     double y_mov = sinl(angle);

//     double distance = sqrt((win_x * win_x) + (win_y * win_y));

//     printf("DA = %f, DX = %f, DY = %f, distance = %f\n", deg_angle, x_mov, y_mov, distance);

//     // if ((win_x > neko_width || win_x < 0) || (win_y > neko_width || win_y < 0)) {
//     //     neko_still = False;
//     // }
//     // printf("X: %d, Y: %d\n", win_x, win_y);
//     // printf("NX: %d, NY: %d\n", neko_x, neko_y);
// }


void get_neko_pos(Display *disp, Window win, _Bool relative, int *neko_x, int *neko_y) {
    Window root_ret, child_ret;
    int root_x, root_y, win_x, win_y;
    unsigned int mask_ret;

    _Bool x_ret = XQueryPointer(
        disp, win, &root_ret, &child_ret, &root_x, 
        &root_y, &win_x, &win_y, &mask_ret
    );

    if (!x_ret) {
        fprintf(stderr, "XQueryPointer: Pointer not on same screen\n");
        return;
    }

    if (relative) {
        *neko_x = win_x;
        *neko_y = win_y;
        return;
    }

    unsigned int width_ret, height_ret, border_width_ret, dept_ret;
    XGetGeometry(
        disp, win, &root_ret, neko_x, neko_y, 
        &width_ret, &height_ret, &border_width_ret, 
        &dept_ret
    );


}

void get_cursor_pos(Display *disp, Window win, int *cursor_x, int *cursor_y) {
    Window root_ret, child_ret;
    int win_x, win_y;
    unsigned int mask_ret;

    _Bool x_ret = XQueryPointer(
        disp, win, &root_ret, &child_ret, cursor_x, 
        cursor_y, &win_x, &win_y, &mask_ret
    );
}

void calc_relative(Display *disp, Window win, struct neko_diff *diff) {
    get_neko_pos(disp, win, True, &(diff->rel_x), &(diff->rel_y));
    // diff->rel_x -= neko_width / 2;
    // diff->rel_y -= neko_height ;
    // printf("rel_x = %d, rel_y = %d\n", diff->rel_x, diff->rel_y);

}

void calc_dxy(Display *disp, Window win, int *x_move, int *y_move) {
    double speed = neko_speed;
    printf("%f\n", speed);
    int neko_x, neko_y;
    int cursor_x, cursor_y;
    int dx, dy;
    double distance;
    double nrml_dx, nrml_dy;    
    double ratio;

    get_neko_pos(disp, win, False, &neko_x, &neko_y);
    get_cursor_pos(disp, win, &cursor_x, &cursor_y);

    dx = cursor_x - neko_x;
    dy = cursor_y - neko_y;


    distance = sqrt((dx * dx) + (dy * dy));
    nrml_dx = dx / distance;
    // printf("%f\n", distance);
    if (distance > speed / 2) {
        ratio = speed / distance;
        printf("ratio %f\n", ratio);
        *x_move = ratio * dx;
        *y_move = ratio * dy;

    } else {
        *x_move = 0;
        *y_move = 0;
    }
}


void neko_move(Display *disp, Window win, XWindowChanges *change) {
    int neko_x, neko_y;
    get_neko_pos(disp, win, False, &neko_x, &neko_y);
    struct neko_diff diff;
    calc_relative(disp, win, &diff);
    if (change->x == 0 && change->y == 0) {
        change->x = neko_x;
        change->y = neko_y;
    }
    // change->x += 0.13 * diff.rel_x;
    // change->y += 0.13 * diff.rel_y;
    int x_move; int y_move;
    calc_dxy(disp, win, &x_move, &y_move);
    change->x += x_move;
    change->y += y_move;

    XConfigureWindow(disp, win, move_value_mask, change);
}



// void neko_change_state(Disp *disp, Window win) {
//     if
// }


// void neko_angle(double x_mov, double y_mov) {
//     if (y_mov ) {
//         if (x_mov )
//     } 
// }


void change_state() {
    if (!neko_still && (neko_state == IDLE || neko_state == KAKI || neko_state == JARE || neko_state == SLEEP)) {
        neko_state = AWAKE;
    }
}



int main() {
    Display *disp;
    Window root_win;
    int screen;
    GC gc;

    disp = XOpenDisplay((char *)0);
    screen = DefaultScreen(disp);
    root_win = create_win(disp);

    // XSynchronize(disp, True);

    set_hints(disp, root_win);

    gc = create_gc(disp, root_win);

    init_anim_map(disp);
    neko_animate(disp, root_win, gc);
    XMapWindow(disp, root_win);  



    // struct timespec tim, tim2;
    // tim.tv_sec = 0;
    // tim.tv_nsec = 125000000;

    neko_state = LEFT;
    anim_start = False;

    XWindowChanges win_change;
    win_change.x = 0;
    win_change.y = 0;
        


    XEvent event;
    int change_x, change_y;
    change_x = 1, change_y = 1;

    for ( ;; ) {
        neko_move(disp, root_win, &win_change);
        // calc_dxy(disp, root_win);
        neko_animate(disp, root_win, gc);
        // sleep_idle_anim(disp, root_win, gc, which);
        // nanosleep(&tim, &tim2);

        while (XPending(disp)) {
            XNextEvent(disp, &event);
            switch (event.type) {
            case Expose:
                if (event.xexpose.count == 0) {
                }
                break;
            case ButtonPress:
                if (event.xbutton.button == Button1) {
                    XFreeGC(disp, gc);
                    XDestroyWindow(disp, root_win);
                    XCloseDisplay(disp);
                    exit(1);
                }
                break;

            // case VisibilityNotify:
            // if (raise_win_delay==0) {
            //   XRaiseWindow(disp,root_win);
            //   raise_win_delay=RAISE_WIN;
            // }
            // break;
            }
        }
    }

    return 0;
}
