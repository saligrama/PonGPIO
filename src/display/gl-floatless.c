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

void gl_draw_pixel(unsigned int x, unsigned int y, color_t c) {
    if (!is_in_bounds(x, y)) return;

    color_t (*pixels)[fb_get_pitch() / fb_get_depth()] = fb_get_draw_buffer();

    unsigned char a = (c & 0xff000000) >> 24;

    // skip the calculations if we are just overwriting the pixel
    pixels[y][x] = c;
    return;
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

unsigned int gl_get_char_height(void) {
    return font_get_glyph_height();
}

unsigned int gl_get_char_width(void) {
    return font_get_glyph_width();
}
