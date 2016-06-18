#include <pebble.h>
#include <inttypes.h>

#include "stat.h"
#include "stat_window.h"

#define MENU_CELL_HEIGHT 35

#define HP_KEY 2
#define WS_KEY 4
#define RAGE_KEY 6

#define HP_DEFAULT 20

static Window *s_main_window;
static MenuLayer *s_menu_layer;
static TextLayer *s_title_layer;

static struct stat s_hp = {
    .cur = HP_DEFAULT,
    .max = HP_DEFAULT,
    .label = "HP"
};

static struct stat s_ws = {
    .cur = 2,
    .max = 2,
    .label = "WS"
};

static struct stat s_rage = {
    .cur = 2,
    .max = 2,
    .label = "Rage"
};

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context)
{
    return 3;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context)
{
    struct stat *stat;
    switch (cell_index->row) {
        case 0: stat = &s_hp; break;
        case 1: stat = &s_ws; break;
        case 2: stat = &s_rage; break;
        default: return;
    }
    static char s_body_text[18];
    snprintf(s_body_text, sizeof(s_body_text), "%s: %"PRIi32, stat->label, stat->cur);
    menu_cell_basic_draw(ctx, cell_layer, s_body_text, NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
    return MENU_CELL_HEIGHT;
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context)
{
    switch (cell_index->row) {
        case 0: stat_window_push(&s_hp); break;
        case 1: stat_window_push(&s_ws); break;
        case 2: stat_window_push(&s_rage); break;
        default: break;
    }
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    
    s_menu_layer = menu_layer_create(bounds);
    menu_layer_set_click_config_onto_window(s_menu_layer, window);
    menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
        .get_num_rows = get_num_rows_callback,
        .draw_row = draw_row_callback,
        .get_cell_height = get_cell_height_callback,
        .select_click = select_callback,
    });
    layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
    
    const GEdgeInsets message_insets = {.top = 145};
    s_title_layer = text_layer_create(grect_inset(bounds, message_insets));
    text_layer_set_text_alignment(s_title_layer, GTextAlignmentCenter);
    text_layer_set_text(s_title_layer, "Stats");
    layer_add_child(window_layer, text_layer_get_layer(s_title_layer));
}

static void window_unload(Window *window)
{
    menu_layer_destroy(s_menu_layer);
    text_layer_destroy(s_title_layer);
}

static void init(void)
{
    read_stat(HP_KEY, &s_hp);
    read_stat(WS_KEY, &s_ws);
    read_stat(RAGE_KEY, &s_rage);

    s_main_window = window_create();

    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load   = window_load,
        .unload = window_unload
    });

    window_stack_push(s_main_window, true);
}

static void deinit(void)
{
    save_stat(HP_KEY, &s_hp);
    save_stat(WS_KEY, &s_ws);
    save_stat(RAGE_KEY, &s_rage);
    
    window_destroy(s_main_window);
}

int main(void)
{
    init();
    app_event_loop();
    deinit();
}
