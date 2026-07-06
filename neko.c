#include "neko.h"

#define neko_width 32
#define neko_height 32

#define SECOND 1000 
#define PI 3.141592654

#define ANGLE_DIFF 20.0
#define UP_ANGLE 270.0
#define RIGHT_ANGLE 360.0
#define DOWN_ANGLE 90.0
#define LEFT_ANGLE 180.0


// TODO: Set cursors, improve structure, add color, improve angle logic
// mouse_cursor.xbm


static int w_depth;
static uint32_t move_value_mask = CWX | CWY;
static XWindowChanges win_change;


static char doc[] = "Oneko recreation in Xlib/C";
static char args_doc[] = "NEKOCCHI";

static struct argp_option options[] = {
    {"speed", 's', "SPEED", 0, "neko speed (default: 13)"},
    {"time", 't', "TIME", 0, "time in milliseconds (default: 125)"},
    { 0 },
};

struct arguments {
    char *args[2];
    char *speed;
    char *time;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) { 
    struct arguments *arguments = state->input;

    switch (key) {
    case 's':
        arguments->speed = arg;
        break;
    case 't':
        arguments->time = arg;
        break;

    case ARGP_KEY_ARG:
        if (state->arg_num >= 1) {
            argp_usage(state);
        }

        arguments->args[state->arg_num] = arg;
        break;
    case ARGP_KEY_END:
        // if (state->arg_num < 0) {
        //     argp_usage(state);
        // }
        break;
        
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}


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

Pixmap cursor_map;
Pixmap cursor_mask;


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
} State;

struct still_state {
    State state;
    int count_time;
    State next_state;
};

struct still_state still_states[5] = {
    {IDLE, NEKO_IDLE_TIME, JARE}, {JARE, NEKO_JARE_TIME, KAKI},
    {KAKI, NEKO_KAKI_TIME, AKUBI}, {AKUBI, NEKO_AKUBI_TIME, SLEEP},
    {AWAKE, NEKO_AWAKE_TIME, IDLE}
};

// debug
const char *state_str[] = {
    ENUM_STR(IDLE),
    ENUM_STR(JARE),
    ENUM_STR(KAKI),
    ENUM_STR(AKUBI),
    ENUM_STR(AWAKE)
};

typedef struct {
    int neko_speed;
    int neko_time;
    State neko_state;
    uint16_t tick_count;
    uint16_t state_count;
} Neko;

Neko neko = {
    .neko_speed = 13, .neko_time = 125, 
    .neko_state = AWAKE, .tick_count = 0, 
    .state_count = 0
};

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

static struct neko_coord coords;

static _Bool anim_start = False;
static _Bool neko_still = True;

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

void free_pixmaps(Display *disp) {
    size_t len = sizeof(maps) / sizeof(maps[0]);
    for (size_t i = 0; i < len; i++) {
        XFreePixmap(disp, *(maps[i].xmp));
        XFreePixmap(disp, *(maps[i].mask));
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
    assert(wm_hints != NULL);
    XSizeHints *wm_size = XAllocSizeHints();
    assert(wm_size != NULL);

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

void set_cursor(Display *disp, Window win, Cursor *cursor) {
    int screen = DefaultScreen(disp);
    XColor sc_black, ex_black;
    XColor sc_white, ex_white;

    
    cursor_mask = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), mouse_cursor_mask_bits, mouse_cursor_mask_width, mouse_cursor_mask_height, 
        1, 0, 1
    );

    cursor_map = XCreatePixmapFromBitmapData(
        disp, RootWindow(disp, screen), mouse_cursor_bits, mouse_cursor_width, mouse_cursor_height, 
        1, 0, 1
    );

    XAllocNamedColor(disp, XDefaultColormap(disp, screen), "black", &sc_black, &ex_black);
    XAllocNamedColor(disp, XDefaultColormap(disp, screen), "white", &sc_white, &ex_white);

    *cursor = XCreatePixmapCursor(
        disp, cursor_map, cursor_mask, 
        &ex_black, &ex_white, 
        mouse_cursor_x_hot, mouse_cursor_y_hot
    );

    XDefineCursor(disp, win, *cursor);
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

void calc_dxy(Display *disp, Window win, int *x_move, int *y_move) {
    double speed = neko.neko_speed;
    int neko_x, neko_y;
    int cursor_x, cursor_y;
    double ratio;

    get_neko_pos(disp, win, False, &neko_x, &neko_y);
    get_cursor_pos(disp, win, &cursor_x, &cursor_y);

    coords.dx = cursor_x - (neko_x + (neko_width / 2));
    coords.dy = cursor_y - (neko_y + (neko_height));

    coords.angle = atan2(coords.dy, coords.dx);
    coords.angle *= 180.0 / PI;

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
    if (coords.angle <= 0) coords.angle = 360 - (-coords.angle);
    printf("%f\n", coords.angle);

    if (coords.angle > UP_ANGLE - ANGLE_DIFF && coords.angle <= UP_ANGLE + ANGLE_DIFF) {
        neko.neko_state = UP;
    } if (coords.angle > UP_ANGLE + ANGLE_DIFF && coords.angle <= RIGHT_ANGLE - ANGLE_DIFF) {
        neko.neko_state = UPRIGHT;
    } if ((coords.angle > RIGHT_ANGLE - ANGLE_DIFF && coords.angle <= RIGHT_ANGLE) || coords.angle <= ANGLE_DIFF){
        neko.neko_state = RIGHT;
    } if (coords.angle > ANGLE_DIFF && coords.angle <= DOWN_ANGLE - ANGLE_DIFF) {
        neko.neko_state = DW_RIGHT;
    } if (coords.angle > DOWN_ANGLE - ANGLE_DIFF && coords.angle <= DOWN_ANGLE + ANGLE_DIFF) {
        neko.neko_state = DOWN;
    } if (coords.angle > DOWN_ANGLE + ANGLE_DIFF && coords.angle <= LEFT_ANGLE - ANGLE_DIFF) {
        neko.neko_state = DW_LEFT;
    } if (coords.angle > LEFT_ANGLE - ANGLE_DIFF && coords.angle <= LEFT_ANGLE + ANGLE_DIFF) {
        neko.neko_state = LEFT;
    } if (coords.angle > LEFT_ANGLE + ANGLE_DIFF && coords.angle <= UP_ANGLE - ANGLE_DIFF) {
        neko.neko_state = UPLEFT;
    }
}

void change_state(State ch_state) {
    neko.state_count = 0;
    neko.tick_count = 0;

    neko.neko_state = ch_state;
}

void poll_neko() {
    struct pollfd ufd;
    ufd.fd = -1;
    ufd.events = POLLIN;

    poll(&ufd, 1, neko.neko_time);
}

void neko_animate(Display *disp, Window win, GC gc) {
    XShapeCombineMask(disp, win, ShapeBounding, 0, 0, *(move_states[neko.neko_state][(int)anim_start].mask), ShapeSet);
    XCopyPlane(disp, *(move_states[neko.neko_state][(int)anim_start].xmp), win, gc, 0, 0, neko_width, neko_height, 0, 0, 1);
    XFlush(disp);

    if (neko.neko_state == SLEEP) {
        anim_start = (neko.tick_count >> 2) & 0x1;
    } else {
        anim_start = neko.tick_count & 0x1;
    }
    neko.state_count += (neko.tick_count & 0x1);

    neko.tick_count++;

    poll_neko();
}

void neko_move(Display *disp, Window win, int x_move, int y_move) {
    int neko_x, neko_y;
    get_neko_pos(disp, win, False, &neko_x, &neko_y);

    if (win_change.x == 0 && win_change.y == 0) {
        win_change.x = neko_x;
        win_change.y = neko_y;
    }

    // int x_move; int y_move;
    // calc_dxy(disp, win, &x_move, &y_move);
    calc_angle();
    win_change.x += x_move;
    win_change.y += y_move;

    XConfigureWindow(disp, win, move_value_mask, &win_change);
    XFlush(disp);
}

ssize_t get_state_index(State check_state) {
    size_t len = sizeof(still_states) / sizeof(still_states[0]);
    // printf("%d\n", len);
    for (size_t i = 0; i < len; i++) {
        // printf("i = %d val = %s\n", i, state_str[i]);
        if (still_states[i].state == check_state) {
            // printf("i = %d, state = %s\n", i , state_str[i]);
            // printf("%d\n", still_states[2].count_time);
            return i;
        }
    }
    fprintf(stderr, "State not in still_states\n");
    return -1;
}

void state_timing(Display *disp, Window win, int x_move, int y_move) {
    _Bool move = x_move == 0 && y_move == 0;
    if (neko.neko_state == IDLE || neko.neko_state == JARE ||  
        neko.neko_state == KAKI  || neko.neko_state == AKUBI || 
        neko.neko_state == SLEEP) {
        if (!move && neko.neko_state != AWAKE) {
            change_state(AWAKE);
        }
    }

    switch (neko.neko_state) {
    case IDLE:
    case JARE:
    case KAKI:
    case AKUBI:
    case AWAKE:
        ssize_t ind = get_state_index(neko.neko_state);
        assert(ind != -1);

        if (neko.state_count < still_states[ind].count_time) break;
        if (move) change_state(still_states[ind].next_state);
        if (neko.neko_state == AWAKE && !move) neko_move(disp, win, x_move, y_move);
        break;

    case DOWN:
    case DW_LEFT:
    case DW_RIGHT:
    case LEFT:
    case RIGHT:
    case UP:
    case UPLEFT:
    case UPRIGHT:
        if (move) change_state(IDLE);
        if (!move) neko_move(disp, win, x_move, y_move);
        break; 

    case SLEEP:
        break;
    default:
        change_state(AWAKE);
        break;
    }
}

void free_resources(Display *disp, Window root_win, GC gc, Cursor cursor) {
    XFreeGC(disp, gc);
    free_pixmaps(disp);
    XFreePixmap(disp, cursor_map);
    XFreePixmap(disp, cursor_mask);
    XFreeCursor(disp, cursor);
    XDestroyWindow(disp, root_win);
    XCloseDisplay(disp);
    exit(1);
}

void event_handler(Display *disp, Window root_win, GC gc, XEvent event, Cursor cursor) {
    while (XPending(disp)) {
        XNextEvent(disp, &event);
        switch (event.type) {
        case Expose:
            if (event.xexpose.count == 0) {
            }
            break;
        case ButtonPress:
            if (event.xbutton.button == Button1) {
                free_resources(disp, root_win, gc, cursor);
            }
            break;
        case VisibilityNotify:
            break;
        }
    }
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv) {
    struct arguments arguments;

    arguments.speed = "13";
    arguments.time = "125";

    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    
    neko.neko_speed = atoi(arguments.speed);
    neko.neko_time = atoi(arguments.time);

    Display *disp;
    Window root_win;
    GC gc;

    Cursor cursor;

    assert((disp = XOpenDisplay((char *)0)) != NULL);
    root_win = create_win(disp);
    set_hints(disp, root_win);
    gc = create_gc(disp, root_win);

    set_cursor(disp, root_win, &cursor);

    init_anim_map(disp);
    neko_animate(disp, root_win, gc);
    XMapWindow(disp, root_win);  

    neko.neko_state = IDLE;
    anim_start = False;

    win_change.x = 0;
    win_change.y = 0;

    int x_move, y_move;
    calc_dxy(disp, root_win, &x_move, &y_move);

    XEvent event;

    for ( ;; ) {
        neko_animate(disp, root_win, gc);
        state_timing(disp, root_win, x_move, y_move);
        calc_dxy(disp, root_win, &x_move, &y_move);

        event_handler(disp, root_win, gc, event, cursor);
    }

    return 0;
}
