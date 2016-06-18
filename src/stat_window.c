#include <pebble.h>
#include <inttypes.h>

#include "stat_window.h"

#define REPEAT_INTERVAL_MS 100

#define MODE_CUR 0
#define MODE_MAX 1

static struct stat *stat;

static Window *s_main_window;
static ActionBarLayer *s_action_bar;
static TextLayer *s_header_layer, *s_value_layer, *s_separator_layer, *s_max_layer;
static GBitmap *s_icon_plus, *s_icon_minus;

static uint16_t s_mode = MODE_CUR;

static void update_text()
{
    static char s_cur_text[18];
    snprintf(s_cur_text, sizeof(s_cur_text), "%"PRIi32, stat->cur);
    text_layer_set_text(s_value_layer, s_cur_text);
    
    static char s_max_text[18];
    snprintf(s_max_text, sizeof(s_max_text), "%"PRIi32, stat->max);
    text_layer_set_text(s_max_layer, s_max_text);
}

static void increment_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch (s_mode) {
        case MODE_CUR:
            if (stat->cur < stat->max)
                stat->cur += 1;
            break;
        case MODE_MAX:
            stat->max += 1;
            break;
        default: break;
    }
    update_text();
}

static void decrement_click_handler(ClickRecognizerRef recognizer, void *context)
{
    switch (s_mode) {
        case MODE_CUR:
            if (stat->cur > -1*stat->max)
                stat->cur -= 1;
            break;
        case MODE_MAX:
            if (stat->max > 0)
                stat->max -= 1;
            break;
        default: break;
    }
    update_text();
}

static void select_click_handler(ClickRecognizerRef recognizer, void *context)
{
    s_mode = (s_mode+1) % 2;
    switch (s_mode) {
        case MODE_CUR:
            text_layer_set_background_color(s_value_layer, GColorBlack);
            text_layer_set_text_color(s_value_layer, GColorWhite);
            text_layer_set_background_color(s_max_layer, GColorWhite);
            text_layer_set_text_color(s_max_layer, GColorBlack);
            break;
        case MODE_MAX:
            text_layer_set_background_color(s_value_layer, GColorWhite);
            text_layer_set_text_color(s_value_layer, GColorBlack);
            text_layer_set_background_color(s_max_layer, GColorBlack);
            text_layer_set_text_color(s_max_layer, GColorWhite);
            break;
        default: break;
    }
}

static void click_config_provider(void *context)
{
    window_single_repeating_click_subscribe(BUTTON_ID_UP,   REPEAT_INTERVAL_MS, increment_click_handler);
    window_single_repeating_click_subscribe(BUTTON_ID_DOWN, REPEAT_INTERVAL_MS, decrement_click_handler);
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

static void window_load(Window *window)
{
    Layer *window_layer = window_get_root_layer(window);

    s_icon_plus  = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_PLUS);
    s_icon_minus = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_ACTION_ICON_MINUS);
    
    s_mode = MODE_CUR;
    
    s_action_bar = action_bar_layer_create();
    action_bar_layer_add_to_window(s_action_bar, window);
    action_bar_layer_set_click_config_provider(s_action_bar, click_config_provider);

    action_bar_layer_set_icon(s_action_bar, BUTTON_ID_UP,   s_icon_plus);
    action_bar_layer_set_icon(s_action_bar, BUTTON_ID_DOWN, s_icon_minus);

    int width = layer_get_frame(window_layer).size.w - ACTION_BAR_WIDTH - 5;

    s_header_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(0, 30), width, 60));
    text_layer_set_font(s_header_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    text_layer_set_background_color(s_header_layer, GColorClear);
    text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
    text_layer_set_text(s_header_layer, stat->label);
    layer_add_child(window_layer, text_layer_get_layer(s_header_layer));

    s_value_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(44, 60), width-2, 38));
    text_layer_set_font(s_value_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(s_value_layer, GColorBlack);
    text_layer_set_text_color(s_value_layer, GColorWhite);
    text_layer_set_text_alignment(s_value_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_value_layer));

    s_separator_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(56, 60), width, 60));
    text_layer_set_font(s_separator_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(s_separator_layer, GColorClear);
    text_layer_set_text_alignment(s_separator_layer, GTextAlignmentCenter);
    text_layer_set_text(s_separator_layer, "_______");
    layer_add_child(window_layer, text_layer_get_layer(s_separator_layer));

    s_max_layer = text_layer_create(GRect(4, PBL_IF_RECT_ELSE(88, 60), width-2, 38));
    text_layer_set_font(s_max_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    text_layer_set_background_color(s_max_layer, GColorClear);
    text_layer_set_text_color(s_max_layer, GColorBlack);
    text_layer_set_text_alignment(s_max_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_max_layer));

    update_text();
}

static void window_unload(Window *window)
{
    text_layer_destroy(s_header_layer);
    text_layer_destroy(s_value_layer);
    text_layer_destroy(s_separator_layer);
    text_layer_destroy(s_max_layer);
    
    action_bar_layer_destroy(s_action_bar);
    
    gbitmap_destroy(s_icon_plus);
    gbitmap_destroy(s_icon_minus);
    
    window_destroy(window);
    s_main_window = NULL;
}

void stat_window_push(struct stat *_stat)
{
    stat = _stat;
    
    if (!s_main_window) {
        s_main_window = window_create();
        window_set_window_handlers(s_main_window, (WindowHandlers) {
            .load   = window_load,
            .unload = window_unload
        });
    }

    window_stack_push(s_main_window, true);
}
