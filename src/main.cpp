#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "stdio.h"
#include <stdlib.h>
#include <math.h>

#define BG_COLOR      0, 0, 0, 0
#define FRAME_W      64
#define FRAME_H      64
#define START_X       5
#define START_Y       0
#define MARGIN_HORIZ  0
#define MARGIN_VERT   0
#define FRAME_COUNT   4

Uint64
get_delta_time()
{
    static Uint64 last_tick_time = 0;
    static Uint64 delta = 0;

    Uint64 now = SDL_GetTicks();
    delta = now - last_tick_time;
    last_tick_time = now;

    return delta;
}

typedef struct Anim {
    Uint64 frame_time;
    Uint64 frame_dur;
    int frame_count;
    int curr_frame;
    SDL_Texture *sheet;
    SDL_FRect *quads;
    SDL_FRect pos;
} Anim;

Anim
make_anim(SDL_Texture *sheet, int frame_count,
          int qx, int qy, int qw, int qh,
          float px, float py, int scale)
{
    Anim a;
    a.frame_count = frame_count;
    a.curr_frame = 0;
    a.frame_time = 0;
    a.frame_dur = 90;
    a.sheet = sheet;
    a.quads = (SDL_FRect*)malloc(sizeof(*a.quads) * a.frame_count);
    int x = qx, y = qy;
    float w = (float)qw, h = (float)qh;
    for (int i=0; i < a.frame_count; i++) {
        a.quads[i].x = (float)x;
        a.quads[i].y = (float)y;
        a.quads[i].w = w;
        a.quads[i].h = h;
        x += qw;
    }
    a.pos = {px, py, (float)qw*scale, (float)qh*scale};
    return a;
}

Anim*
draw_anim(SDL_Renderer *rend, Anim *a)
{
    //printf("%g %g\n", a->pos.x, a->pos.y);
    SDL_RenderTexture(rend, a->sheet, a->quads + a->curr_frame, &(a->pos));
    return a;
}

void
update_anim(Anim *a, Uint64 dt)
{
    a->frame_time += dt;
    a->curr_frame = a->frame_time / a->frame_dur;
    if (a->curr_frame >= a->frame_count) {
        a->curr_frame %= a->frame_count;
        a->frame_time %= a->frame_dur * a->frame_count;
    }
}

int
main(void)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS) != 1) {
        printf("SDL_Init failed: ");
        printf("%s\n", SDL_GetError());
        return 1;
    }

    SDL_DisplayID display_id = SDL_GetPrimaryDisplay();

    SDL_Rect display_bounds;
    if (SDL_GetDisplayUsableBounds(display_id, &display_bounds) != 1) {
        printf("SDL_GetDisplayBounds error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "blorp",
        display_bounds.w,
        display_bounds.h,
        SDL_WINDOW_TRANSPARENT | SDL_WINDOW_MAXIMIZED |
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_UTILITY |
        SDL_WINDOW_KEYBOARD_GRABBED | SDL_WINDOW_MOUSE_GRABBED |
        SDL_WINDOW_HIGH_PIXEL_DENSITY
    );

    if (!window) {
        printf("SDL_CreateWindow failed: ");
        printf("%s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    float FRAME_SCALE = SDL_GetWindowDisplayScale(window);
    display_bounds.w *= FRAME_SCALE;
    display_bounds.h *= FRAME_SCALE;

    SDL_Renderer *rend = SDL_CreateRenderer(window, NULL);
    if (!rend) {
        printf("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_SetDefaultTextureScaleMode(rend, SDL_SCALEMODE_NEAREST);
    SDL_SetRenderDrawBlendMode(rend, SDL_BLENDMODE_BLEND);
    SDL_SetRenderTarget(rend, NULL);

    SDL_Surface *image = IMG_Load("spritesheet.png");
    if (!image) {
        printf("IMG_Load error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Texture *tex = SDL_CreateTextureFromSurface(rend, image);
    SDL_DestroySurface(image);
    if (tex == NULL) {
        printf("SDL_CreateTextureFromSurface error: %s\n", SDL_GetError());
        return 1;
    }
    Uint64 now_ms = SDL_GetTicks();

    printf("dpy bounds (x,y,w,h): %i %i %i %i\n",
           display_bounds.x, display_bounds.y, display_bounds.w, display_bounds.h);
    int start_x = START_X + display_bounds.x;
    int start_y = START_Y; // + display_bounds.y;
    int horiz_count = ceil((display_bounds.w - start_x) / (FRAME_W * FRAME_SCALE + MARGIN_HORIZ));
    int vert_count = ceil((display_bounds.h - start_y) / (FRAME_H * FRAME_SCALE + MARGIN_VERT));
    printf("vert, horiz = %i, %i\n", vert_count, horiz_count);
    Anim *anims = (Anim*) malloc(horiz_count * vert_count * sizeof(Anim));
    int x = start_x, y = start_y;
    for (int i=0; i < horiz_count * vert_count; i++) {
        anims[i] = make_anim(
            tex,
            FRAME_COUNT,
            0, 0, FRAME_W, FRAME_H,
            x, y, FRAME_SCALE);
        anims[i].frame_time = i * anims[i].frame_dur * 0.8;
        x += FRAME_W*FRAME_SCALE + MARGIN_HORIZ;
        if (x >= display_bounds.x + display_bounds.w) {
            x = START_X;
            y += FRAME_H*FRAME_SCALE + MARGIN_VERT;
            //if (y >= display_bounds.h) break;
        }
    }

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("fixedsys.ttf", 72);
    if (!font) SDL_Log("Failed to load font: %s", SDL_GetError());

    SDL_SetRenderScale(rend, 1.0f, 1.0f);
    //SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");  // Nearest neighbor

    const char lines[] = {
        "Don't open\nweird files\nfrom the internet!!!"
    };

    Uint64 guy_count = 0;
    Uint64 guy_count_inc_every = 120;
    Uint64 guy_count_time_leftover = 0;

    int quit = 0;
    while (!quit) {
        Uint64 dt = get_delta_time();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = 1;
            } else if (event.type == SDL_EVENT_KEY_DOWN) {
                switch (event.key.key) {
                case SDLK_ESCAPE:
                    quit = 1;
                    break;
                case SDLK_Q:
                    quit = 1;
                    break;
                }
            }
        }

        guy_count_time_leftover += dt;
        if (guy_count_time_leftover >= guy_count_inc_every) {
            guy_count_time_leftover -= guy_count_inc_every;
            guy_count += 1;
            guy_count_inc_every -= 5;
            if (guy_count_inc_every < 30) guy_count_inc_every = 30;
            if (guy_count >= horiz_count * vert_count)
                guy_count = horiz_count * vert_count;
        }

        for (int i=0; i < horiz_count * vert_count; i++) {
            update_anim(anims + i, dt);
        }
        SDL_SetRenderDrawColor(rend, BG_COLOR);
        SDL_RenderClear(rend);
        for (int i=0; i < guy_count; i++) {
            draw_anim(rend, anims + i);
        }

        SDL_FRect center_rect = {0};
        center_rect.w = 820.0;
        center_rect.h = 300.0;
        center_rect.x = (display_bounds.x + display_bounds.w)/2 - center_rect.w/2;
        center_rect.y = (display_bounds.y + display_bounds.h)/2 - center_rect.h/2;
        if (guy_count >= horiz_count * vert_count) {
            SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
            SDL_RenderFillRect(rend, &center_rect);

            float x = center_rect.x + 30;
            float y = center_rect.y + 30;
            for (int i = 0; i < strlen(lines); i++) {
                if (lines[i] == '\n') {y += 72; x = center_rect.x + 30; continue; }
                SDL_Surface *surf = TTF_RenderGlyph_Solid(font, lines[i], (SDL_Color){255, 255, 255, 255});
                SDL_Texture *text = SDL_CreateTextureFromSurface(rend, surf);
                SDL_FRect dst = {
                    x,
                    y,
                    (float)surf->w,
                    (float)surf->h,
                };
                SDL_RenderTexture(rend, text, NULL, &dst);
                SDL_DestroySurface(surf);
                SDL_DestroyTexture(text);
                x += dst.w + 5;
            }
        }

        SDL_RenderPresent(rend);
        if (dt < 16)
            SDL_Delay(16-dt);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
