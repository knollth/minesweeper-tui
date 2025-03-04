//#define TB_IMPL // not needed if termbox2.h included first
#include "termbox2.h"
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define CELL_IS_MINE    0x01  // 00000001
#define CELL_DISCOVERED 0x02  // 00000010
#define CELL_FLAGGED    0x04  // 00000100


#define IS_DISCOVERED(cell) ((cell.flags & CELL_DISCOVERED) != 0)

#define MAX_HEIGHT      24
#define MAX_WIDTH       30
//#define MAX_QUEUE_SIZE  2*(MAX_WIDTH+MAX_HEIGHT)+50
#define MAX_QUEUE_SIZE  (MAX_WIDTH*MAX_HEIGHT)


// ----------------- UI -----------------

typedef struct {
    int width;
    int height;
    int offset_x;
    int offset_y;
} BoxCoordinates;


// ----------------- Game -----------------
//
typedef enum game_action {DEFAULT, QUIT, FLAG, REVEAL, UP, DOWN, LEFT, RIGHT} game_action;
typedef enum game_status {ONGOING, LOST, WON, ABORT} game_status;

typedef struct {
    uint8_t flags;
    uint8_t adjMines;
} CellData;

typedef struct {
    int width;
    int height;
    int mine_count;
    uint16_t flag_count;
    uint16_t num_discovered;
    CellData** grid;
    game_status status;
} GameData;

typedef struct {
    int width;
    int height;
    int mine_count;
} GameSettings;

typedef struct {
    uint8_t x;
    uint8_t y;
} CellCoords;

typedef struct {
    CellCoords data[MAX_QUEUE_SIZE];
    int front;
    int rear;
    int count;
} Queue;
uint16_t get_num_discovered(GameData* g);

void flood_fill_discover(uint16_t x, uint16_t y, GameData* g);
void init_queue(Queue* q);
void enqueue(Queue* q, int x, int y);
CellCoords dequeue(Queue* q);

void allocate_game_grid(GameData* g);
void free_game_grid(GameData *game);

void place_mines(GameData* g);
void move_mine(uint16_t x, uint16_t y, GameData *g);
void change_adj_minecounts(uint16_t x, uint16_t y, short delta, GameData* g);
void update_adjcell_minecount(uint16_t x, uint16_t y, GameData* g);

inline wchar_t get_status_emoji(game_status s);

uint8_t reveal_cell(uint16_t x, uint16_t y, GameData* g);
uint8_t chord_cell(uint16_t x, uint16_t y, GameData* g);

uint8_t get_adj_flagged_cells(uint16_t x, uint16_t y, GameData* g);
void flag_cell(uint16_t x, uint16_t y, GameData* g);


void game_loop(GameData* g);


void draw_game(uint16_t curX, uint16_t curY, GameData* g);
void draw_display_grid(GameData* g, uintattr_t fg, uintattr_t bg);
void draw_debug_info(uint16_t x, uint16_t y, GameData* g);
void draw_cell_contents(GameData* g);

inline uintattr_t get_fg_color(uint8_t adjMines);

uint8_t new_game_dialogue(GameData* g);
uint8_t quit_dialogue(GameData *g);

inline game_action get_action(struct tb_event ev);


int get_display_grid_x(int x);
int get_display_grid_y(int y);

int get_display_grid_height(int height);
int get_display_grid_width(int width);


//static inline int get_center_x_offset(int width);
//static inline int get_center_y_offset (int height);

void center_box(BoxCoordinates* b, int rel_width, int rel_height);



uint8_t make_game_selection(GameData* g);
GameSettings make_custom_selection(BoxCoordinates b);
void draw_custom_settings_menu(BoxCoordinates b, uint8_t selection);
void draw_selection_menu(BoxCoordinates b, uint8_t selection, GameSettings opts[], uint8_t options_size);
void draw_box(BoxCoordinates b, uintattr_t fg, uintattr_t bg);
void clear_box_content(BoxCoordinates b);
void init_game();

