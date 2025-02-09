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

void draw_mines(int startx, int starty, GameData* g);

void draw_grid(int startx, int starty, int cellcols, int cellrows, uintattr_t fg, uintattr_t bg);
int get_termgrid_x(int x);
int get_termgrid_y(int y);

int get_termgrid_height(int height);
int get_termgrid_width(int width);


int get_center_x_offset(int width);
int get_center_y_offset (int height);


void make_game_selection(GameData* g);
GameSettings make_custom_selection(BoxCoordinates b);
void draw_custom_settings_menu(BoxCoordinates b, uint8_t selection);
void draw_selection_menu(BoxCoordinates b, uint8_t selection, GameSettings opts[], uint8_t options_size);
void draw_box(BoxCoordinates b, uintattr_t fg, uintattr_t bg);
void clear_box_content(BoxCoordinates b);
void init_game();

