#include "gl.h"
#include "font.h"
#include <stdbool.h>
#include "rpi.h"

// only include system.h if it is included when compiling
// Add -lpiextra to LDLIBS and compile using the macro
// HAS_SYSTEM_H to use caches
#ifdef HAS_SYSTEM_H
#include "system.h"
#endif


void gl_init(unsigned int width, unsigned int height, gl_mode_t mode) {
    fb_init(width, height, 4, mode);    // use 32-bit depth always for graphics library

    
    // only enable caches if system.h is included
    #ifdef HAS_SYSTEM_H
    system_enable_cache();
    printf("using caches to speed up graphics library\n");
    #endif
    
}

void gl_swap_buffer(void) {
    fb_swap_buffer();
}

unsigned int gl_get_width(void) {
    return fb_get_width();
}

unsigned int gl_get_height(void) {
    return fb_get_height();
}

color_t gl_color(unsigned char r, unsigned char g, unsigned char b) {
    return (0xff << 24) + (r << 16) + (g << 8) + b;
}

void gl_clear(color_t c) {
    color_t *pixels = fb_get_draw_buffer();
    unsigned int size = fb_get_height() * fb_get_pitch() / fb_get_depth();
    for (int i = 0; i < size; i++) {
        pixels[i] = c;
    }
}

static bool is_in_bounds(unsigned int x, unsigned int y) {
    return (x >= 0 && x < fb_get_pitch() / fb_get_depth()) && (y >= 0 && y < fb_get_height());
}

static int interpolate(int old_val, int new_val, float percent) {
    return (int) (old_val * (1.0f - percent)) + (int) (new_val * percent);
}

void gl_draw_pixel(unsigned int x, unsigned int y, color_t c) {
    if (!is_in_bounds(x, y)) return;

    color_t (*pixels)[fb_get_pitch() / fb_get_depth()] = fb_get_draw_buffer();

    unsigned char a = (c & 0xff000000) >> 24;

    // skip the calculations if we are just overwriting the pixel
    if (a == 255) {
        pixels[y][x] = c;
        return;
    }
    
    unsigned char new_r = (c & 0xff0000) >> 16;
    unsigned char new_g = (c & 0xff00) >> 8;
    unsigned char new_b = (c & 0xff);
    
    color_t curr_color = pixels[y][x];
    unsigned char curr_r = (curr_color & 0xff0000) >> 16;
    unsigned char curr_g = (curr_color & 0xff00) >> 8;
    unsigned char curr_b = (curr_color & 0xff);

    float a_percent = a / 255.0f;
    color_t interpolated_color = gl_color(
            interpolate(curr_r, new_r, a_percent),
            interpolate(curr_g, new_g, a_percent),
            interpolate(curr_b, new_b, a_percent));
    pixels[y][x] = interpolated_color;
}

color_t gl_read_pixel(unsigned int x, unsigned int y) {
    if (!is_in_bounds(x, y)) return 0;

    color_t (*pixels)[fb_get_pitch() / fb_get_depth()] = fb_get_draw_buffer();
    return pixels[y][x];
}

void gl_draw_rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, color_t c) {
    if (!is_in_bounds(x, y)) return;

    unsigned int bot_right_x = is_in_bounds(x+w, y) ? x+w : fb_get_pitch() / fb_get_depth();
    unsigned int bot_right_y = is_in_bounds(x, y+h) ? y+h : fb_get_height();

    for (unsigned int i = y; i < bot_right_y; i++) {
        for (unsigned int j = x; j < bot_right_x; j++) {
            gl_draw_pixel(j, i, c);
        }
    }
}

void gl_draw_char(unsigned int x, unsigned int y, char ch, color_t c) {
    unsigned char buf[font_get_glyph_size()];
    bool did_get_glyph = font_get_glyph(ch, buf, sizeof(buf));
    if (!did_get_glyph) return;

    unsigned char (*glyph)[gl_get_char_width()] = &buf;

    unsigned int width = gl_get_char_width();
    unsigned int height = gl_get_char_height();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (glyph[i][j] == 0xff) gl_draw_pixel(x + j, y + i, c);
        }
    }
}

void gl_draw_string(unsigned int x, unsigned int y, const char* str, color_t c) {
    size_t len = strlen(str);
    for (int i = 0; i < len; i++) {
        gl_draw_char(x + (i * gl_get_char_width()), y, str[i], c);
    }
}

static void swap(unsigned int *a, unsigned int *b) {
    unsigned int tmp = *a;
    *a = *b;
    *b = tmp;
}

void gl_draw_line(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, color_t c) {
    float slope;
    bool is_steep;
    bool is_vert = false;
    if (x1 == x2) {
        slope = 0;
        is_steep = true;
        is_vert = true;
    } else {
        slope = ((float) y2 - y1) / ((float) x2 - x1);
        float abs_slope = slope < 0 ? -slope : slope;
        is_steep = abs_slope > 1;
    }

    gl_draw_pixel(x1, y1, c);
    gl_draw_pixel(x2, y2, c);

    if (is_steep) {
        // swap x and y for calculation purposes
        // when displaying, swap them again
        swap(&x1, &y1);
        swap(&x2, &y2);
        
        if (!is_vert) slope = 1 / slope;
    }

    // make (x1, y1) the left point and (x2, y2) the right point
    if (x1 > x2) {
        // swap the points
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    for (unsigned int i = 1; i < x2 - x1; i++) {
        float actual_y = y1 + (slope * i);
        unsigned int y_new_1 = (unsigned int) actual_y;
        unsigned int y_new_2 = (unsigned int) (actual_y + 1);
        
        unsigned int alpha_1 = 255 * (1 - (actual_y - y_new_1));
        unsigned int alpha_2 = 255 * (1 - (y_new_2 - actual_y));

        color_t c1 = (c & 0xffffff) | (alpha_1 << 24);
        color_t c2 = (c & 0xffffff) | (alpha_2 << 24);

        if (is_steep) {
            gl_draw_pixel(y_new_1, x1 + i, c1);
            gl_draw_pixel(y_new_2, x1 + i, c2);
        } else {
            gl_draw_pixel(x1 + i, y_new_1, c1);
            gl_draw_pixel(x1 + i, y_new_2, c2);
        }
    }
}

static unsigned int get_x_from_line_y(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int y) {
    if (y2 == y1) {
        return x1; // y->x is not a function, so return arbitrary value
    }
    if (y2 < y1) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    
    float inverse_slope = ((float) x2 - x1) / ((float) y2 - y1);
    return x1 + (int) (inverse_slope * (y - y1));
}

void gl_draw_triangle(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned int x3, unsigned int y3, color_t c) {
    // sort the points so y1 <= y2 <= y3
    if (y2 < y1) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }
    if (y3 < y1) {
        swap(&x1, &x3);
        swap(&y1, &y3);
    }
    if (y3 < y2) {
        swap(&x2, &x3);
        swap(&y2, &y3);
    }

    gl_draw_line(x1, y1, x2, y2, c);
    gl_draw_line(x2, y2, x3, y3, c);
    gl_draw_line(x1, y1, x3, y3, c);
    for (unsigned int y = y1 + 1; y < y3; y++) {
        unsigned int x_boundary_1 = get_x_from_line_y(x1, y1, x3, y3, y);
        bool boundary_1_goes_left = x3 < x1;

        unsigned int x_boundary_2;
        bool boundary_2_goes_left;
        if (y >= y2) {
            x_boundary_2 = get_x_from_line_y(x2, y2, x3, y3, y);
            boundary_2_goes_left = x3 < x2;
        } else {
            x_boundary_2 = get_x_from_line_y(x1, y1, x2, y2, y);
            boundary_2_goes_left = x2 < x1;
        }

        unsigned int x_min, x_max;
        // the left side of the line should start 1 pixel late if the left boundary line goes right
        // the right side of the line should end 1 pixel early if the right boundary line goes left
        int x_min_shift, x_max_shift;
        if (x_boundary_1 < x_boundary_2) {
            x_min = x_boundary_1;
            x_min_shift = boundary_1_goes_left ? 0 : 1;
            x_max = x_boundary_2;
            x_max_shift = boundary_2_goes_left ? -1 : 0;
        } else {
            x_min = x_boundary_2;
            x_min_shift = boundary_2_goes_left ? 0 : 1;
            x_max = x_boundary_1;
            x_max_shift = boundary_1_goes_left ? -1 : 0;
        }

        if (x_max - x_min > 1)
            // horizontal lines, so no anti-aliasing
            gl_draw_line(x_min + x_min_shift, y, x_max + x_max_shift, y, c);
    }
}

unsigned int gl_get_char_height(void) {
    return font_get_glyph_height();
}

unsigned int gl_get_char_width(void) {
    return font_get_glyph_width();
}
