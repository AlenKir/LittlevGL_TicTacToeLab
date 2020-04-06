#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>
#include "lvgl/lvgl.h"

//-----------------------------------------------------------------------------
// Display wrapper structures and functions
//-----------------------------------------------------------------------------

/**
 * Display wrapper.
 */
typedef struct {
    // display width and height
    int width;
    int height;

    // display input device
    int point_x;
    int point_y;
    // 0 - not pressed
    // 1 - pressed
    int point_state;

    SDL_Window *window;
    SDL_Renderer *renderer;
} demo_display_t;

lv_obj_t *buttons[9]; // buttons
lv_obj_t *labels[9]; // ...
int button_pressed;
bool symbol; //true - x, false - o

int demo_display_init(demo_display_t *demo_disp, int width, int height);
int demo_display_deinit(demo_display_t *monitor);
int demo_display_process_events(demo_display_t *demo_disp);
int demo_display_fill_rect(demo_display_t *demo_disp, int x1, int y1, int x2, int y2, uint16_t *colors);
int demo_display_draw_rect(demo_display_t *demo_disp, int x1, int y1, int x2, int y2, uint16_t color);

static void event_handler(lv_obj_t * obj, lv_event_t event)
{
    if(event == LV_EVENT_PRESSED)
    {
        for (int i = 0; i < 9; i++)
        {
        	if (lv_btn_get_state(buttons[i]) == LV_BTN_STATE_PR){
        		printf("Clicked on the %d button \n", i+1);
        		if (symbol)
        			lv_label_set_text(labels[i], "x");
        		else
        			lv_label_set_text(labels[i], "o");
        		symbol = !symbol;
        	}
        }
    }
}

/**
 * Initialize SDL and demo display.
 *
 * @param demo_disp uninitialized demo_display_t structure
 * @param width window width
 * @param height window height
 * @return 0 on success, otherwise non-zero value
 */
int demo_display_init(demo_display_t *demo_disp, int width, int height) {
    // initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return -1;
    }

    // create window
    demo_disp->width = width;
    demo_disp->height = height;
    demo_disp->window = SDL_CreateWindow("Tic Tac Toe", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (demo_disp->window == NULL) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return -1;
    }

    // create renderer for window
    demo_disp->renderer = SDL_CreateRenderer(demo_disp->window, -1, SDL_RENDERER_ACCELERATED);
    if (demo_disp->renderer == NULL) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return -1;
    }

    // reset mouse state
    demo_disp->point_x = 0;
    demo_disp->point_y = 0;
    demo_disp->point_state = 0;

    return 0;
}

/**
 * Free
 */
int demo_display_deinit(demo_display_t *demo_disp) {
    // destroy rendered and window
    SDL_DestroyRenderer(demo_disp->renderer);
    SDL_DestroyWindow(demo_disp->window);

    // stop SDL subsystem
    SDL_Quit();

    return 0;
}

/**
 * Process SDL events.
 *
 * The function store mouse events and ignore other ones.
 *
 * @param demo_disp display
 * @return 0 on success, otherwise non-zero value
 */
int demo_display_process_events(demo_display_t *demo_disp) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // check mouse events
        switch (event.type) {
        case SDL_MOUSEMOTION:
            // save coordinates
            demo_disp->point_x = event.motion.x;
            demo_disp->point_y = event.motion.y;
            break;
        case SDL_MOUSEBUTTONUP:
            demo_disp->point_state = 0;
            break;
        case SDL_MOUSEBUTTONDOWN:
            demo_disp->point_state = 1;
            break;
        case SDL_QUIT:
            return -1;
            break;
        default:
            // ignore other events
            break;
        }
    }
    return 0;
}

/**
 * Fill display with data from specified array.
 *
 * @param demo_disp
 * @param x1 left area coordinate
 * @param x2 rigth area coordinate
 * @param y1 top area coordinate
 * @param y2 bottom area coordinate
 * @param colors array with area colors
 * @return 0 on success, otherwise non-zero value
 */
int demo_display_fill_rect(demo_display_t *demo_disp, int x1, int y1, int x2, int y2, uint16_t *colors) {
    int tmp;
    int err;
    int i;
    uint16_t color;
    uint8_t r, g, b;

    if (x1 > x2) {
        tmp = x2;
        x2 = x1;
        x1 = tmp;
    }
    if (y1 > y2) {
        tmp = y2;
        y2 = y1;
        y1 = tmp;
    }

    i = 0;
    for (int y = y1; y <= y2; y++) {
        for (int x = x1; x <= x2; x++) {

            color = colors[i];
            r = (color & 0xF800) >> 8; // rrrrr... ........ -> rrrrr000
            g = (color & 0x07E0) >> 3;  // .....ggg ggg..... -> gggggg00
            b = (color & 0x1F) << 3;  // ............bbbbb -> bbbbb000;

            err = SDL_SetRenderDrawColor(demo_disp->renderer, r, g, b, SDL_BLENDFACTOR_DST_ALPHA);
            if (err) {
                fprintf(stderr, "SDL_SetRenderDrawColor Error: %s\n", SDL_GetError());
                return -1;
            }
            err = SDL_RenderDrawPoint(demo_disp->renderer, x, y);
            if (err) {
                fprintf(stderr, "SDL_RenderDrawPoint Error: %s\n", SDL_GetError());
                return -1;
            }
            i++;
        }
    }

    // "flush" data
    SDL_RenderPresent(demo_disp->renderer);

    return 0;
}

//-----------------------------------------------------------------------------
// LittlevGL driver port
//-----------------------------------------------------------------------------

/**
 * LittlevGL driver wrapper.
 */
typedef struct {
    demo_display_t *demo_disp;

    lv_color_t *disp_buf_1;
    lv_disp_buf_t lvgl_disp_buf;

    lv_disp_drv_t lvgl_disp_drv;
    lv_indev_drv_t lvgl_indev_drv;
} demo_lvgl_disp_t;

int demo_lvgl_init(demo_lvgl_disp_t *demo_lvgl_disp, demo_display_t *demo_disp);
int demo_lvgl_deinit(demo_lvgl_disp_t *demo_lvgl_disp);

static void demo_lvgl_flush_cb(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
{
    demo_lvgl_disp_t *demo_lvgl_disp = disp_drv->user_data;
    demo_display_t *demo_disp = demo_lvgl_disp->demo_disp;

    // flush to monitor
    demo_display_fill_rect(demo_disp, area->x1, area->y1, area->x2, area->y2, (uint16_t*) color_p);

    // notify LittlevGL that data has been flushed
    lv_disp_flush_ready(disp_drv);
}

static bool demo_lvgl_indev_cb(lv_indev_drv_t *indev_drv, lv_indev_data_t *data)
{
    demo_lvgl_disp_t *demo_lvgl_disp = indev_drv->user_data;
    demo_display_t *demo_disp = demo_lvgl_disp->demo_disp;

    if (demo_disp->point_state > 0) {
        data->state = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = demo_disp->point_x;
    data->point.y = demo_disp->point_y;

    return false;
}

/**
 * Initialize LittlevGL and bind it to demo_disp.
 *
 * @param demo_lvgl_disp littlevgl driver wrapper
 * @param demo_disp display driver wrapper
 * @return 0 on success, otherwise non-zero value
 */
int demo_lvgl_init(demo_lvgl_disp_t *demo_lvgl_disp, demo_display_t *demo_disp) {
    // initialize lvgl
    lv_init();

    demo_lvgl_disp->demo_disp = demo_disp;

    // allocate internal and initialize lvgl buffer
    size_t buf_size = demo_disp->width * 10;
    demo_lvgl_disp->disp_buf_1 = malloc(buf_size * sizeof(lv_color_t));
    lv_disp_buf_init(&demo_lvgl_disp->lvgl_disp_buf, demo_lvgl_disp->disp_buf_1, NULL, buf_size);

    // prepare and initialize display
    lv_disp_drv_init(&demo_lvgl_disp->lvgl_disp_drv);
    demo_lvgl_disp->lvgl_disp_drv.buffer = &demo_lvgl_disp->lvgl_disp_buf;
    demo_lvgl_disp->lvgl_disp_drv.flush_cb = demo_lvgl_flush_cb;
    demo_lvgl_disp->lvgl_disp_drv.user_data = demo_lvgl_disp;
    lv_disp_t *disp;
    disp = lv_disp_drv_register(&demo_lvgl_disp->lvgl_disp_drv);
    if (disp == NULL) {
        // error
        return -1;
    }

    // prepare input driver
    lv_indev_drv_init(&demo_lvgl_disp->lvgl_indev_drv);
    demo_lvgl_disp->lvgl_indev_drv.user_data = demo_lvgl_disp;
    demo_lvgl_disp->lvgl_indev_drv.disp = disp;
    demo_lvgl_disp->lvgl_indev_drv.type = LV_INDEV_TYPE_POINTER;
    demo_lvgl_disp->lvgl_indev_drv.read_cb = demo_lvgl_indev_cb;
    lv_indev_t *indev = lv_indev_drv_register(&demo_lvgl_disp->lvgl_indev_drv);
    if (indev == NULL) {
        // process error
        return -1;
    }

    return 0;
}

/**
 * Deinitialize  LittlevGL driver wrapper.
 *
 * @param demo_lvgl_disp littlevgl driver wrapper
 * @return 0 on success, otherwise non-zero value
 */
int demo_lvgl_deinit(demo_lvgl_disp_t *demo_lvgl_disp) {
    free(demo_lvgl_disp->disp_buf_1);
    return 0;
}

int main(int n, char **args) {
    int err;
    int ret_code = 0;
    demo_display_t demo_disp;
    int width = 320;
    int height = 240;
    demo_lvgl_disp_t demo_lvgl_diplay;

    // prepare window
    err = demo_display_init(&demo_disp, width, height);
    if (err) {
        return -1;
    }

    // initialize lvgl
    err = demo_lvgl_init(&demo_lvgl_diplay, &demo_disp);
    if (err) {
        return -1;
    }

    symbol = true;

    // coord caclulation
    int coord_x = -120;
    int coord_y = -130;
    for (int i = 0; i < 9; i++)
    {
        buttons[i] = lv_btn_create(lv_scr_act(), NULL);
        lv_obj_set_event_cb(buttons[i], event_handler);
        lv_obj_align(buttons[i], NULL, LV_ALIGN_CENTER, coord_x, coord_y);
        if (i == 2 | i == 5) {
        	coord_x = -120;
        	coord_y = coord_y + 75;
        }
        else
        	coord_x = coord_x + 105;
        labels[i] = lv_label_create(buttons[i], NULL);
        lv_label_set_text(labels[i], " ");
    }

    uint32_t t, t_prev;
    t_prev = SDL_GetTicks();

    while (1) {
        // read SDL events
        err = demo_display_process_events(&demo_disp);
        if (err) {
            break;
        }

        // process lvgl timer
        t = SDL_GetTicks();
        lv_tick_inc(t - t_prev);
        t_prev = t;

        // process lvgl events
        lv_task_handler();

        // loop delay
        SDL_Delay(5);
    }

    // close window
    demo_lvgl_deinit(&demo_lvgl_diplay);
    err = demo_display_deinit(&demo_disp);
    if (err) {
        return -1;
    }

    return ret_code;
}
