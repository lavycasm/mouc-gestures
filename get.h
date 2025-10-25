#define BREAKPOINT_THRESHOLD 1.25f

// or differnces between each position?
POINT get_deviations(size_t start, size_t stop) {
    POINT avg_dev = {0};

    for(size_t i = start; i < stop - 1; ++i) {
        avg_dev.x += positions[i].x - positions[i + 1].x;
        avg_dev.y += positions[i].y - positions[i + 1].y;
    }

    avg_dev.x /= (LONG)(stop-start);
    avg_dev.y /= (LONG)(stop-start);

    return avg_dev;
}

void get_min_and_max(size_t start, size_t stop, POINT* out_min, POINT* out_max) {
    POINT min = {.x = 9999, .y = 9999};
    POINT max = {.x =    0, .y =    0};

    for(size_t i = start; i < stop; ++i) {
        if (positions[i].x > max.x) max.x = positions[i].x;
        if (positions[i].y > max.y) max.y = positions[i].y;
        if (positions[i].x < min.x) min.x = positions[i].x;
        if (positions[i].y < min.y) min.y = positions[i].y;
    }

    *out_min = min;
    *out_max = max;
}

POINT get_averages(size_t start, size_t stop) {
    POINT avg_pos = {0};
    
    for(size_t i = start; i < stop; ++i) {
        avg_pos.x += positions[i].x;
        avg_pos.y += positions[i].y;
    }

    avg_pos.x /= (LONG)(stop - start);
    avg_pos.y /= (LONG)(stop - start);

    return avg_pos;
}

//returns array of indexes to potential breakpoints / angle changes
size_t* get_breakpoints(size_t* out_size) {
    size_t *breakpoints = calloc(16, sizeof(size_t));
    size_t idx = 0;

    for(size_t i = 1; i < size - 1; ++i) {
        float x0  = positions[i].x - positions[i-1].x;
        float y0  = positions[i].y - positions[i-1].y;
        float x1  = positions[i+1].x - positions[i].x;
        float y1  = positions[i+1].y - positions[i].y;
        float a0 = atan2(y0, x0);
        float a1 = atan2(y1, x1);
        float da = fabs(a0 - a1); 
        if (da > M_PI) da = 2.0 * M_PI - da;
        if (da > BREAKPOINT_THRESHOLD) breakpoints[idx++] = i;
    }

    *out_size = idx;
    return breakpoints;
}

// gets radius from a potential circle gesture
float get_circle_radius(size_t start, size_t stop) {
    POINT avg_pos = get_averages(start, stop);
    POINT min, max;
    get_min_and_max(start, stop, &min, &max);
    float cx = (float)(max.x+min.x)/2.f;
    float cy = (float)(max.y+min.y)/2.f;

    float a = max.x - min.x;
    float b = max.y - min.y;

    float adjust_threshold = max((max.x / avg_pos.x), (max.y / avg_pos.y));

    if (fabsf(a-b) <= CIRCLE_THRESHOLD * adjust_threshold) {
        float r = sqrtf((a + b) / 2.f);
        return r;
    }
    return 0.f;
}

// returns what kind of gesture was made in positions[start..stop]
Gesture get_gesture(size_t start, size_t stop) {
    assert((stop - start) != 0 && "size in get_gesture is zero");
    POINT avg_dev = get_deviations(start, stop);
    POINT avg_pos = get_averages(start, stop);

    float r = get_circle_radius(start, stop);
    if (r > 0.f) {
        return Circle;
    }

    if(abs(avg_dev.x) > abs(avg_dev.y)) {
        if(avg_dev.x < 0)
            return Right;
        else 
            return Left;
    }    
    else {
        if(avg_dev.y < 0) 
            return Down;
        else 
            return Up;
    }
    return 0;
}
