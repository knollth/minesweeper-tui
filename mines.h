//#define TB_IMPL // not needed if termbox2.h included first
#include "termbox2.h"
#include <stdint.h>

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t mine_count;
} GameSettings;

typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t offset_x;
    uint8_t offset_y;

} BoxCoordinates;

typedef struct {
    int isMine;
    uint16_t x;
    uint16_t y;
} CellData;



GameSettings make_game_selection();
GameSettings make_custom_selection(BoxCoordinates b);
void draw_custom_settings_menu(BoxCoordinates b, uint8_t selection);
void draw_selection_menu(BoxCoordinates b, uint8_t selection, GameSettings opts[], uint8_t options_size);
void draw_box(BoxCoordinates b, uintattr_t fg, uintattr_t bg);
void clear_box_content(BoxCoordinates b);
void init_game();
void draw_grid(uint8_t startx, uint8_t starty, uint8_t cellcols, uint8_t cellrows, uintattr_t fg, uintattr_t bg);

