
#define WIN_LEAN_AND_MEAN
#include "windows.h"

#define CIRCLE_THRESHOLD 70.f
#define CIRCLE_RADIUS_SIZE 15
#define POLLING_RATE 20

typedef enum {None, Up, Down, Left, Right, Circle} Gesture;
typedef struct {
    size_t amount;
    char** combo;
    char** cmd;
} configuration;

static LPPOINT positions;
static size_t size;
static char gesture_string[256];
static configuration config;
