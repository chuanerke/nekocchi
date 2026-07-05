#include "neko.h"
#include <signal.h>
#include <poll.h>

#define neko_width 32
#define neko_height 32

#define neko_speed 13
#define SECOND 1000 
#define PI 3.141592654

static int w_depth;
static uint32_t move_value_mask = CWX | CWY;

static int neko_time = 125;
static int tick_count = 0;


static XWindowChanges win_change;



struct neko_diff {
    int rel_x;
    int rel_y;
};

struct neko_coord {
    int dx;
    int dy;
    double distance;
    double angle;
};

static int prev_x, prev_y;


static struct neko_coord coords;


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
    {&upright1, &upright1_mask, upright1_bits, upright1_mask_bits}, {&upright2, &upright2_mask, upright2_bits, upright2_mask_bits},
    {&utogi1, &utogi1_mask, utogi1_bits, utogi1_mask_bits}, {&utogi2, &utogi2_mask, utogi2_bits, utogi2_mask_bits}
};



typedef enum {
    IDLE,
    AKUBI,
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

static int state_count = 0;

struct animation {
    Pixmap *xmp;
    Pixmap *mask;
};

struct animation move_states[][2] = {
    {{&mati2, &mati2_mask}, {&mati2, &mati2_mask}}, 
    {{&mati3, &mati3_mask}, {&mati3, &mati3_mask }},
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
    printf("LEN = %d\n", len);
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

    if (neko_state == SLEEP) {
        anim_start = (tick_count >> 2) & 0x1;
    } else {
        anim_start = tick_count & 0x1;
        printf("%d\n", anim_start);
    }
    tick_count++;
    // state_count += (tick_count & 0x1);
    if ((tick_count % 2) == 0) {
        state_count++;
    }
    printf("state %d\n", state_count);

    poll_neko(disp);
}

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


void change_state(int ch_state) {
    state_count = 0;
    tick_count = 0;

    neko_state = ch_state;
}


void calc_dxy(Display *disp, Window win, int *x_move, int *y_move) {
    double speed = neko_speed;
    int neko_x, neko_y;
    int cursor_x, cursor_y;
    double ratio;

    get_neko_pos(disp, win, False, &neko_x, &neko_y);
    get_cursor_pos(disp, win, &cursor_x, &cursor_y);

    coords.dx = cursor_x - (neko_x + (neko_width / 2));
    coords.dy = cursor_y - (neko_y + (neko_height));

    coords.angle = atan2(coords.dy, coords.dx);
    coords.angle *= 180.0 / PI;
    printf("%f\n", coords.angle);

    coords.distance = sqrt((coords.dx * coords.dx) + (coords.dy * coords.dy));
    if (coords.distance > speed / 2) {
        ratio = speed / coords.distance;
        *x_move = ratio * coords.dx;
        *y_move = ratio * coords.dy;

    } else {
        *x_move = 0;
        *y_move = 0;
    }
}
void calc_angle() {
    if (coords.angle < 25 && coords.angle >= -25) {
        neko_state = RIGHT;
    } else if (coords.angle >= 25 && coords.angle < 55) {
        neko_state = DW_RIGHT;
    } else if (coords.angle >= 55 && coords.angle < 115) {
        neko_state = DOWN;
    } else if (coords.angle >= 115 && coords.angle < 165) {
        neko_state = DW_LEFT;
    } else if ((coords.angle >= 165 && coords.angle <= 179) || (coords.angle <= -179 && coords.angle >= -155)) {
        neko_state = LEFT;
    } else if (coords.angle > -155 && coords.angle <= -115) {
        neko_state = UPLEFT;
    } else if (coords.angle > -115 && coords.angle <= -60) {
        neko_state = UP;
    } else if (coords.angle > -60 && coords.angle < -25) {
        neko_state = UPRIGHT;
    }
    // TODO: do rest properly, two upleft upright, get pixmap error
}

void neko_move(Display *disp, Window win) {
    int neko_x, neko_y;
    get_neko_pos(disp, win, False, &neko_x, &neko_y);

    if (win_change.x == 0 && win_change.y == 0) {
        win_change.x = neko_x;
        win_change.y = neko_y;
    }

    int x_move; int y_move;
    calc_dxy(disp, win, &x_move, &y_move);
    calc_angle();
    win_change.x += x_move;
    win_change.y += y_move;

    XConfigureWindow(disp, win, move_value_mask, &win_change);
    XFlush(disp);
}


void state_timing(Display *disp, Window win, int x_move, int y_move) {
    if (neko_state == IDLE || neko_state == JARE ||  neko_state == KAKI  || neko_state == AKUBI || neko_state == SLEEP) {
        if ((x_move != 0 || y_move != 0) && neko_state != AWAKE) {
            change_state(AWAKE);
        }
    }

    switch (neko_state) {

    case IDLE:
        if (state_count < NEKO_IDLE_TIME) {
            break;
        }
        if (x_move == 0 && y_move == 0) {
            change_state(JARE);
        }
        break;
    case JARE:
        if (state_count < NEKO_JARE_TIME) {
            break;
        }
        if (x_move == 0 && y_move == 0) {
            change_state(KAKI);
        }
        break; 
    case KAKI:
        if (state_count < NEKO_KAKI_TIME) {
            break;
        }
        if (x_move == 0 && y_move == 0) {
            change_state(AKUBI);
        }
        break;
    case AKUBI:
        if (state_count < NEKO_AKUBI_TIME) {
            break;
        }
        if (x_move == 0 && y_move == 0) {
            change_state(SLEEP);
        }
        break;
    case AWAKE:
        if (state_count < NEKO_AWAKE_TIME) {
            break;
        }
        if (x_move == 0 && y_move == 0) {
            change_state(IDLE);
        }
        if (x_move != 0 || y_move != 0) {
            neko_move(disp, win);
        }
        break;
    case DOWN:
    case DW_LEFT:
    case DW_RIGHT:
    case LEFT:
    case RIGHT:
    case UP:
    case UPLEFT:
    case UPRIGHT:
        if (x_move == 0 && y_move == 0) {
            change_state(IDLE);
        }
        if (x_move != 0 || y_move != 0) {
            neko_move(disp, win);
        }
        break;
    case SLEEP:
        break;
    default:
        change_state(AWAKE);
        break;
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

    neko_state = AWAKE;
    anim_start = False;

    win_change.x = 0;
    win_change.y = 0;

    XEvent event;
    int change_x, change_y;
    change_x = 1, change_y = 1;
    int x_move, y_move;
    calc_dxy(disp, root_win, &x_move, &y_move);


    for ( ;; ) {
        neko_animate(disp, root_win, gc);
        state_timing(disp, root_win, x_move, y_move);
        calc_dxy(disp, root_win, &x_move, &y_move);


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
