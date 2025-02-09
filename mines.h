//#define TB_IMPL // not needed if termbox2.h included first
#include "termbox2.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

typedef struct {
    int width;
    int height;
    int offset_x;
    int offset_y;
} BoxCoordinates;


typedef struct {
    uint8_t isMine;
    uint8_t isDiscovered;
    uint8_t isFlagged;
    uint8_t adjMines;
} CellData;

typedef struct {
    int width;
    int height;
    int mine_count;
    CellData** grid;
} GameData;

typedef struct {
    int width;
    int height;
    int mine_count;
} GameSettings;



void free_game_grid(GameData *game);
void allocate_game_grid(GameData* g);
void place_mines(GameData* g);
void incr_adj_minecounts(uint16_t x, uint16_t y, GameData* g);
void update_adjcell_minecount(uint16_t x, uint16_t y, GameData* g);

void draw_mines(GameData* g);
void format_display_cell(uint16_t x, uint16_t y, uint16_t fg, uint16_t bg);
void draw_display_grid(GameData* g, uintattr_t fg, uintattr_t bg);
void draw_cursor(uint16_t x, uint16_t y, GameData* g);

int get_display_grid_x(int x);
int get_display_grid_y(int y);

int get_display_grid_height(int height);
int get_display_grid_width(int width);


//static inline int get_center_x_offset(int width);
//static inline int get_center_y_offset (int height);




void make_game_selection(GameData* g);
GameSettings make_custom_selection(BoxCoordinates b);
void draw_custom_settings_menu(BoxCoordinates b, uint8_t selection);
void draw_selection_menu(BoxCoordinates b, uint8_t selection, GameSettings opts[], uint8_t options_size);
void draw_box(BoxCoordinates b, uintattr_t fg, uintattr_t bg);
void clear_box_content(BoxCoordinates b);
void init_game();

