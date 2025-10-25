#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#define _USE_MATH_DEFINES
#include "math.h"

#include "ini.h"
#include "main.h"
#include "get.h"

static int handler(void* user, const char* section, const char* name,
                   const char* value)
{
    configuration* config = (configuration*)user;

    #define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("gesture", "combo")) {
        config->combo[config->amount] = strdup(value);
    }
    if (MATCH("gesture", "cmd")) {
        config->cmd[config->amount] = strdup(value);
    } else {
        return 0;  /* unknown section/name, error */
    }
    config->amount++;
    return 1;
}

void move_range(size_t* start, size_t* stop, size_t next_stop) {
    *start = *stop;
    *stop = next_stop;
}

void print_info(size_t* breakpoints, size_t breakpoints_size) {
    size_t start = 0, i = 0;
    size_t stop = size;

    printf("------------ GESTURE INFO ------------\n");
    printf("BREAK POINTS [%d]:\n", breakpoints_size);
    if (breakpoints_size > 0) stop = breakpoints[0];
    while(i <= breakpoints_size) {
        printf("positions[%d..%d]: (%d, %d)..(%d, %d)\n", start, stop, positions[start].x, positions[start].y, positions[stop].x, positions[stop].y);
        if (stop - start < 3) {
            printf("\tmight be bogus... skipping\n");
        }

        POINT avg_dev = get_deviations(start, stop);
        POINT avg_pos = get_averages(start, stop);
        POINT min, max; 
        get_min_and_max(start, stop, &min, &max);
        printf("\tavg_pos.x = %-4d, avg_pos.y = %-4d\n", avg_pos.x, avg_pos.y);
        printf("\tavg_dev.x = %-4d, avg_dev.y = %-4d\n", avg_dev.x, avg_dev.y);
        printf("\tmin_pos.x = %-4d, min_pos.y = %-4d\n", min.x, min.y);
        printf("\tmax_pos.x = %-4d, max_pos.y = %-4d\n", max.x, max.y);

        i++; 
        if (i >= breakpoints_size) {
            move_range(&start, &stop, size);
        }
        else if (i < breakpoints_size) {
            move_range(&start, &stop, breakpoints[i]);
        }
    }
    printf("\n");
    printf("Gesture combo: %s\n", gesture_string);
    printf("-------------------------------------\n");
}

void process_gesture() {
    if(size <= 1) return;
    
    size_t start = 0, i = 0, idx = 0;
    size_t stop = size;
    size_t breakpoints_size = 0;
    Gesture gesture_array[16] = {0};
    Gesture radius_array[16] = {0};
    
    size_t* breakpoints = get_breakpoints(&breakpoints_size);
    if (breakpoints_size > 0) stop = breakpoints[0];

    while(i <= breakpoints_size) {
        if (stop - start < 3) {
            goto skip_bogus;
        }

        radius_array[idx] = get_circle_radius(start, stop);
        gesture_array[idx] = get_gesture(start, stop);
        idx++;
        
        skip_bogus:
        i++; 
        if (i >= breakpoints_size) {
            move_range(&start, &stop, size);
        }
        else if (i < breakpoints_size) {
            move_range(&start, &stop, breakpoints[i]);
        }

    };

    strcpy(gesture_string, "");
    for(size_t i = 0; i < idx; ++i) {
        Gesture gesture = gesture_array[i];
        switch(gesture) {
            case None:
                break;
            case Up:
                strcat(gesture_string, "Up");
                break;
            case Down:
                strcat(gesture_string, "Down");
                break;
            case Left:
                strcat(gesture_string, "Left");
                break;
            case Right:
                strcat(gesture_string, "Right");
                break;
            case Circle:
                if (radius_array[i] <= CIRCLE_RADIUS_SIZE) 
                    strcat(gesture_string, "Small ");
                else 
                    strcat(gesture_string, "Large ");
                strcat(gesture_string, "Circle");
                break;
        }
        if (i + 1 != idx) strcat(gesture_string, ", ");
    }

    print_info(breakpoints, breakpoints_size);

    for(size_t i = 0; i < config.amount; ++i) {
        const char* combo = config.combo[i];
        if (strcmp(gesture_string, combo) == 0){
            const char* cmd = config.cmd[i];
            printf("GOT %s\nrunning %s\n", combo, cmd);

            const char* header = "#Requires AutoHotkey v2.0.18+\n\n";
            FILE *temp = fopen("temp.ahk", "wb");
            fwrite(header, sizeof(char), strlen(header), temp);
            fwrite(cmd, sizeof(char), strlen(cmd), temp);
            fclose(temp);

            if(system("temp.ahk /force")) remove("temp.ahk");
        }
    }

    free(breakpoints);

}

void free_config() {
    config.amount = 0;
    free(config.cmd);
    free(config.combo);
}

bool init_config() {
    config = (configuration){
        .amount = 0, 
        .combo = calloc(128, sizeof(256)), 
        .cmd = calloc(128, sizeof(1024))
    };

    if (ini_parse("config.ini", handler, &config) < 0) {
        printf("Can't load 'config.ini'\n");
        return false;
    }
    return true;
}

void clean_up() {
    free(positions);
    free_config();
    remove("temp.ahk");
}

int main() {
    
    atexit(clean_up);

    positions = calloc(2 << 16, sizeof(POINT));
    size = 0;
    if(!init_config()) return 1;

    POINT prev_pos = {0};
    POINT curr_pos = {0};
    bool rmb_is_down = false;
    for(;;) {
        Sleep(POLLING_RATE);
        GetCursorPos(&curr_pos);
        bool new_pos = !(prev_pos.x == curr_pos.x && prev_pos.y == curr_pos.y);
        bool rmb_pressed = GetAsyncKeyState(VK_MBUTTON) & 0x8000;
        bool r_pressed = GetAsyncKeyState('R') & 0x8000;

        if(r_pressed && rmb_pressed) {
            printf("reloading config...\n");
            free_config();
            if(!init_config()) return 1;
            printf("config reloaded!!!\n");
        }
        if (!rmb_pressed && rmb_is_down) {
            rmb_is_down = !rmb_is_down;
            process_gesture();
        }
        if (!rmb_pressed) {
            continue;
        }
        if(!rmb_is_down) {
            rmb_is_down = !rmb_is_down;
            size = 0;
        }

        prev_pos.x = curr_pos.x;
        prev_pos.y = curr_pos.y;
        if(new_pos) {
            positions[size++] = curr_pos;
        }
    }

    return 0;
}
