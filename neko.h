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
#include <math.h>
#include <time.h>
#include <poll.h>
#include <argp.h>
#include <assert.h>

#include "bitmaps/neko/neko.include"
#include "bitmasks/neko/neko.mask.include"

#define NEKO_DEPTH 1

// sourced from oneko-sakura
#define NEKO_IDLE_TIME 4
#define NEKO_JARE_TIME 10
#define NEKO_KAKI_TIME 4
#define NEKO_AKUBI_TIME 6       
#define NEKO_AWAKE_TIME 3
#define NEKO_TOGI_TIME 10

#define ENUM_STR(ENUM_VAL) #ENUM_VAL

