/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2022 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_PLAYDATE

#include <math.h>

#include "../SDL_sysvideo.h"
#include "SDL_playdate_framebuffer_c.h"

#include "pd_api.h"

#define PLAYDATE_SURFACE   "_SDL_PlaydateSurface"

static const uint8_t map[8][8] = {
        { 1, 49, 13, 61, 4, 52, 16, 64 },
        { 33, 17, 45, 29, 36, 20, 48, 32 },
        { 9, 57, 5, 53, 12, 60, 8, 56 },
        { 41, 25, 37, 21, 44, 28, 40, 24 },
        { 3, 51, 15, 63, 2, 50, 14, 62 },
        { 25, 19, 47, 31, 34, 18, 46, 30 },
        { 11, 59, 7, 55, 10, 58, 6, 54 },
        { 43, 27, 39, 23, 42, 26, 38, 22 }
};

int SDL_PLAYDATE_CreateWindowFramebuffer(_THIS, SDL_Window * window, Uint32 * format, void ** pixels, int *pitch)
{
    SDL_Surface *surface;
    const Uint32 surface_format = SDL_PIXELFORMAT_RGB888;
    int w, h;

    /* Free the old framebuffer surface */
    SDL_PLAYDATE_DestroyWindowFramebuffer(_this, window);

    /* Create a new one */
    SDL_GetWindowSize(window, &w, &h);
    surface = SDL_CreateRGBSurfaceWithFormat(0, w, h, 0, surface_format);
    if (!surface) {
        return -1;
    }

    /* Save the info and return! */
    SDL_SetWindowData(window, PLAYDATE_SURFACE, surface);
    *format = surface_format;
    *pixels = surface->pixels;
    *pitch = surface->pitch;

    return 0;
}

static const float r_d = 0.212671f;
static const float g_d = 0.715160f;
static const float b_d = 0.072169f;

int SDL_PLAYDATE_UpdateWindowFramebuffer(_THIS, SDL_Window * window, const SDL_Rect * rects, int numrects)
{
    SDL_Surface *surface;
    surface = (SDL_Surface *) SDL_GetWindowData(window, PLAYDATE_SURFACE);
    if (!surface) {
        return SDL_SetError("Couldn't find surface for window");
    }

    Uint32* pixels = surface->pixels;
    uint8_t* frame = pd->graphics->getFrame();

    int y = 0; int x = 0;
#ifdef PLAYDATE_UNROLL_FB_LOOP
#pragma unroll 96000
#endif
    for (int i = 0; i<96000;i++) {
        if (i % 400 == 0) {
            x = 0;
            if (y < 239) {
                y++;
            }
        }

        Uint8 r; Uint8 g; Uint8 b;
        SDL_GetRGB(pixels[i], surface->format, &r, &g, &b);
        Uint8 cie_intensity_0 = r_d * r + g_d * g + b_d * b;
        int ii = cie_intensity_0 + cie_intensity_0 * map[y % 8][x % 8] / 63;
        ii >= 172 ? (frame[y * LCD_ROWSIZE + (x >> 3)] |= (1 << (7 - (x & 7)))) : (frame[y * LCD_ROWSIZE + (x >> 3)] &= ~(1 << (7 - (x & 7))));
        x++;
    }

    pd->graphics->markUpdatedRows(0,239);

    return 0;
}

void SDL_PLAYDATE_DestroyWindowFramebuffer(_THIS, SDL_Window * window)
{
    SDL_Surface *surface;

    surface = (SDL_Surface *) SDL_SetWindowData(window, PLAYDATE_SURFACE, NULL);
    SDL_FreeSurface(surface);
}

#endif /* SDL_VIDEO_DRIVER_PLAYDATE */

/* vi: set ts=4 sw=4 expandtab: */
