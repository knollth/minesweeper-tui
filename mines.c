#include "mines.h"
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#define DISPLAY_GRID_X(x) (2 + 4 * (x))
#define DISPLAY_GRID_Y(y) (1 + 2 * (y))
#define DISPLAY_GRID_HEIGHT(h) ((h) * 2 + 1)
#define DISPLAY_GRID_WIDTH(w) ((w) * 4 + 1)

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

inline uint8_t isMine(CellData c){
    return (c.flags & CELL_IS_MINE);
}

inline uint8_t isDiscovered(CellData c){
    return (c.flags & CELL_DISCOVERED);
}

inline uint8_t isFlagged(CellData c){
    return (c.flags & CELL_FLAGGED);
}

inline uint16_t display_grid_startx(GameData* g){
    return (tb_width() - DISPLAY_GRID_WIDTH(g->width))/2;
}
inline uint16_t display_grid_starty(GameData* g){
    return (tb_height() - DISPLAY_GRID_HEIGHT(g->height))/2;
}

// Box-drawing Unicode characters (using Nerd Fonts)
const uint32_t top_left_round = 0x256D;   // '╭'
const uint32_t top_right_round = 0x256E;  // '╮'
const uint32_t bottom_left_round = 0x2570; // '╰'
const uint32_t bottom_right_round = 0x256F; // '╯'
const uint32_t horizontal = 0x2500;  // '─'
const uint32_t vertical = 0x2502;    // '│'
const uint32_t vert_b = 0x2503;    // '┃'
//
const uint32_t top_left = 0x250C;    // '┌'
const uint32_t top_right = 0x2510;   // '┐'
const uint32_t bottom_left = 0x2514; // '└'
const uint32_t bottom_right = 0x2518; // '┘'
const uint32_t intersection = 0x253C; // '┼'
const uint32_t top_intersection = 0x252C; // '┬'
const uint32_t bottom_intersection = 0x2534; // '┴'
const uint32_t intersection_left = 0x251C; // '├'
const uint32_t intersection_right = 0x2524; // '┤'
//
wchar_t flag = L'⚑';
wchar_t flag_emoji = L'🚩';
wchar_t mine = L'💣';

int main() {
    //struct tb_event ev;
    //int y = 0;

    tb_init();
    setlocale(LC_ALL, ""); /* Fix unicode handling */
    srand(time(NULL));
    init_game();


    //tb_poll_event(&ev);


    tb_shutdown();

    return 0;
}

void init_game(){
    struct tb_event ev;
    GameData g = {};
    make_game_selection(&g);
    allocate_game_grid(&g);
    if(g.grid == NULL){
        tb_clear();
        tb_printf(0, 0, 0, 0, "error allocating grid");
        tb_present();
        tb_poll_event(&ev);
        return;
    }
    tb_clear();

    draw_display_grid(&g, 0, 0);

    place_mines(&g);
    draw_mines(&g);
    start_game_loop(&g);

    free_game_grid(&g);


    return;
}
void start_game_loop(GameData* g){
    struct tb_event ev;
    CellData* curCell;

    uint16_t curX = 0;
    uint16_t curY = 0;

    enum game_state curState = ONGOING;
    enum game_action curAction;

    uint8_t first_turn = true;

    while(curState == ONGOING){
        tb_clear();
        draw_display_grid(g, 0, 0);
        draw_cursor(curX, curY, g);
        draw_cell_info(curX, curY, g);
        draw_control_info(0, tb_height()-1);
        draw_mines(g);

        curCell = &g->grid[curY][curX];
        tb_poll_event(&ev);

        curAction = get_action(ev);
        if (ev.type != TB_EVENT_KEY) continue;

        switch(curAction){
        case LEFT:
            (curX <= 0) ? curX=g->width-1 : curX--;
            break;
        case RIGHT:
            (curX >= g->width-1)? curX=0 : curX++;
            break;
        case DOWN:
            (curY >= g->height-1)? curY=0 : curY++;
            break;
        case UP:
            (curY <= 0) ? curY = g->height-1: curY--;
            break;
        case FLAG: 
            flag_cell(curX, curY, g);
            break;
        case QUIT: 
            if(quit_dialogue(g)) curState = ABORT;
            break;
        case REVEAL:
            if(first_turn && isMine(*curCell)) {
                move_mine(curX, curY, g);
            }
            first_turn = false;
            if(reveal_cell(curX, curY, g)){
                curState = LOST;
            }
            break;
        default:
                break;
        }
    }
    tb_clear();
    draw_display_grid(g, 0, 0);
    draw_finished(g);
    tb_poll_event(&ev);

}

inline game_action get_action(struct tb_event ev) {
    if (ev.key) { 
        switch (ev.key) {
            case TB_KEY_ESC: return QUIT;
            case TB_KEY_SPACE: return FLAG;
            case TB_KEY_ENTER: return REVEAL;
            case TB_KEY_ARROW_UP: return UP;
            case TB_KEY_ARROW_LEFT: return LEFT;
            case TB_KEY_ARROW_DOWN: return DOWN;
            case TB_KEY_ARROW_RIGHT: return RIGHT;
            default: break;
        }
    } else { 
        switch (ev.ch) {
            case 'q': return QUIT;
            case 'f': return FLAG;
            case 'w': case 'k': return UP;
            case 'a': case 'h': return LEFT;
            case 's': case 'j': return DOWN;
            case 'd': case 'l': return RIGHT;
            default: break;
        }
    }
    return DEFAULT;
}

void allocate_game_grid(GameData* g){
    //g->grid = malloc(g->height*sizeof(CellData*));
    g->grid = calloc(g->height,sizeof(CellData*));
    if(!g->grid){
        tb_clear();
        tb_printf(0, 0, 0, 0, "Error Allocating Minesweeper Grid Buffer");
        return;
    }
    for(int y = 0; y < g->height;y++){
        //g->grid[y] = malloc(g->width * sizeof(CellData));
        g->grid[y] = calloc(g->width, sizeof(CellData));
        if (!g->grid[y]) {
            for (int j = 0; j < y; j++) free(g->grid[j]);
            free(g->grid);
            g->grid = NULL;
            return;
        }
    }
}

void free_game_grid(GameData *game) {
    if (game->grid) {
        for (int i = 0; i < game->height; i++) {
            free(game->grid[i]);
        }
        free(game->grid);
        game->grid = NULL;
    }
}
// ----------------- Cell Handling/ Operations-----------------

uint8_t reveal_cell(uint16_t x, uint16_t y, GameData* g){
    CellData *c = &g->grid[y][x];
    uint16_t adjFlags = get_adj_flagged_cells(x,y,g);

    assert(c->adjMines <= 8 && c->adjMines >= 0);
    
    if(isFlagged(*c))
        return false;
    if(c->flags & CELL_IS_MINE){
        return true;
    }
    if(c->adjMines > 0 
        && adjFlags == c->adjMines 
        && isDiscovered(*c)){
        return chord_cell(x, y, g);
    }
    if( c->adjMines == 0 ){
        flood_fill_discover(x, y, g);
    } 
    c->flags |= CELL_DISCOVERED;
    return false;
}


uint8_t chord_cell(uint16_t x, uint16_t y, GameData* g){
    assert("cell must be within game grid" &&
           y < g->height && x < g->width);

    uint16_t lower_y = (y == 0) ? y: y-1; 
    uint16_t upper_y = (y == g->height-1) ? y: y+1; 
    uint16_t lower_x = (x == 0) ? x: x-1; 
    uint16_t upper_x = (x == g->width-1) ? x: x+1; 

    CellData* c;

    for(int i = lower_y; i <= upper_y; i++){
        for (int j = lower_x; j <= upper_x; j++){
            if (i == y && j == x) continue;
            c = &(g->grid[i][j]);
            if(isFlagged(*c)) continue;

            if(isMine(*c))
                return true;
            if(c->adjMines == 0){
                flood_fill_discover(j, i, g);
            }
            c->flags |= CELL_DISCOVERED;
        }
    }
    return false;
}



uint8_t get_adj_flagged_cells(uint16_t x, uint16_t y, GameData* g){
    assert("cell must be within game grid" &&
           y < g->height && x < g->width);

    uint16_t lower_y = (y == 0) ? y: y-1; 
    uint16_t upper_y = (y == g->height-1) ? y: y+1; 
    uint16_t lower_x = (x == 0) ? x: x-1; 
    uint16_t upper_x = (x == g->width-1) ? x: x+1; 

    uint8_t adjFlags = 0;

    for(int i = lower_y; i <= upper_y; i++){
        for (int j = lower_x; j <= upper_x; j++){
            if (i == y && j == x) continue;
            if(g->grid[i][j].flags & CELL_FLAGGED) 
                adjFlags++;
        }
    }
    return adjFlags;
}



void flag_cell(uint16_t x, uint16_t y, GameData* g){
    CellData c = g->grid[y][x];
    if(isDiscovered(c))
        return;
    if(!isFlagged(c)){
        if(g->flag_count+1 > g->mine_count) 
            return;
        g->flag_count++;
    } else {
        g->flag_count--;
    }
    g->grid[y][x].flags ^= CELL_FLAGGED ;
}
void place_mines(GameData* g){
    int n = g->width * g->height;
    CellData* c;
    uint16_t x;
    uint16_t y;


    for(int count = 0; count < g->mine_count;){
        int i = rand() % n;
        x = i%(g->width);
        y = i/(g->width);

        c = &g->grid[y][x];

        if(!(c->flags & CELL_IS_MINE)){
            c->flags |= CELL_IS_MINE;
            change_adj_minecounts(x, y, 1, g);
            count++;
        }
    }
}

void move_mine(uint16_t x, uint16_t y, GameData *g){
    assert("cell must be within game grid" &&
           y < g->height && x < g->width);
    int n = g->width * g->height;

    uint16_t new_x = x;
    uint16_t new_y = y;

    while(g->grid[new_y][new_x].flags & CELL_IS_MINE) {
        int i = rand() % n;
        new_x = i%(g->width);
        new_y = i/(g->width);
    };
    change_adj_minecounts(x, y, -1, g);
    g->grid[y][x].flags &= ~(1 << (CELL_IS_MINE-1));
    change_adj_minecounts(new_x, new_y, 1, g);
    g->grid[new_y][new_x].flags &= ~(1 << (CELL_IS_MINE-1));
}

void change_adj_minecounts(uint16_t x, uint16_t y, short delta, GameData* g){
    assert("cell must be within game grid" &&
           y < g->height && x < g->width && delta >= 0 && delta );

    uint16_t lower_y = (y == 0) ? y: y-1; 
    uint16_t upper_y = (y == g->height-1) ? y: y+1; 

    uint16_t lower_x = (x == 0) ? x: x-1; 
    uint16_t upper_x = (x == g->width-1) ? x: x+1; 

    for(int i = lower_y; i <= upper_y; i++){
        for (int j = lower_x; j <= upper_x; j++){
            if (i == y && j == x) 
                continue;
            g->grid[i][j].adjMines += delta;
        }
    }
}


// --------------------- Queue ---------------------

void flood_fill_discover(uint16_t x, uint16_t y, GameData* g){
    assert("cell must be within game grid" && 
           y < g->height && x < g->width);

    Queue q;
    CellCoords c;

    init_queue(&q);
    enqueue(&q, x, y);

    g->grid[y][x].flags |= CELL_DISCOVERED;
    g->num_discovered++;

    while(q.count > 0){
        c = dequeue(&q);

        if(g->grid[c.y][c.x].adjMines > 0){
            continue;
        }

        uint8_t upper_y = (c.y == g->height-1) ? c.y: c.y+1; 
        uint8_t lower_y = (c.y == 0) ? c.y: c.y-1; 
        uint8_t upper_x = (c.x == g->width-1) ? c.x: c.x+1; 
        uint8_t lower_x = (c.x == 0) ? c.x: c.x-1; 

        for(int i = lower_y; i <= upper_y; i++){
            for (int j = lower_x; j <= upper_x; j++){
                if (i == c.y && j == c.x)
                    continue;
                
                if (!(g->grid[i][j].flags & CELL_DISCOVERED) &&
                    !(g->grid[i][j].flags & CELL_IS_MINE)){
                    g->grid[i][j].flags |= CELL_DISCOVERED;
                    g->num_discovered++;
                    enqueue(&q, j, i);
                }
            }
        }
    }
}


void init_queue(Queue* q){
    q->front = 0;
    q->rear = -1;
    q->count = 0;
}

void enqueue(Queue* q, int x, int y){
    q->rear = (q->rear + 1) % MAX_QUEUE_SIZE;
    q->data[q->rear].x = x; 
    q->data[q->rear].y = y; 
    q->count++;
}

CellCoords dequeue(Queue* q){
    CellCoords c = q->data[q->front];
    q->front = (q->front+1)%MAX_QUEUE_SIZE;
    q->count--;
    return c;
}

// ----------------- Game Rendering -----------------
//
void draw_control_info(uint16_t startx, uint16_t starty){
    //tb_printf(startx, starty-1, 0, 0, "[ left: h ]  [ right: l ] [ up: k ] [ down: j ]");
    tb_printf(startx, starty, 0, 0, "[ quit: q ]  [ reveal: ENTER ] [ flag: f ]" );
}

void draw_cell_info(uint16_t x, uint16_t y, GameData* g){
    uint16_t startx = display_grid_startx(g);
    uint16_t starty = display_grid_starty(g);

    uint16_t disp_x = startx+DISPLAY_GRID_X(x);
    uint16_t disp_y = starty+DISPLAY_GRID_Y(y);

    CellData c = g->grid[y][x];

    char isMine = (c.flags & CELL_IS_MINE) ? 'y' : 'n';
    char isDiscovered = (c.flags & CELL_DISCOVERED) ? 'y' : 'n';
    char isFlagged = (c.flags & CELL_FLAGGED) ? 'y' : 'n';

    tb_printf(0, 0, 0, 0, "tb_height: %d | tb_width: %d ", tb_height(), tb_width());
    tb_printf(0, 1, 0, 0, "curX: %d | curY: %d | gridx: %d | gridy: %d", x, y ,disp_x, disp_y );
    tb_printf(0, 2, 0, 0, "startx: %d | starty: %d | disp_x: %d | disp_y %d", startx, starty, disp_x, disp_y);
    tb_printf(0, 3, 0, 0, "isMine: %c | isFlagged: %c | isDiscovered: %c ", isMine, isFlagged, isDiscovered);
    tb_printf(0, 4, 0, 0, "flag_count: %d | discovered_cell_count: %d", g->flag_count, g->num_discovered );
}

void draw_cursor(uint16_t x, uint16_t y, GameData* g){
    //CellData c = g->grid[y][x];
    uint16_t startx = display_grid_startx(g);
    uint16_t starty = display_grid_starty(g);

    uint16_t disp_x = startx+DISPLAY_GRID_X(x);
    uint16_t disp_y = starty+DISPLAY_GRID_Y(y);
    
    tb_set_cursor(disp_x, disp_y);
    tb_present();
}


void draw_mines(GameData* g){
    uint16_t startx = display_grid_startx(g);
    uint16_t starty = display_grid_starty(g);

    for(int y=0; y < g->height; y++){
        for(int x=0; x < g->width; x++){
            CellData c = g->grid[y][x];

            uint16_t disp_x = startx+DISPLAY_GRID_X(x);
            uint16_t disp_y = starty+DISPLAY_GRID_Y(y);


            tb_set_cell(disp_x, disp_y, ' ' , 0, 0);
            if(c.flags & CELL_DISCOVERED){
                if(c.flags & CELL_IS_MINE){
                    tb_set_cell(disp_x,disp_y,mine, 0, 0);
                } else if(c.adjMines > 0){
                    tb_printf(disp_x,disp_y,0,0,"%d", c.adjMines);
                }
           } else {
                tb_set_cell(disp_x, disp_y, ' ', 0, TB_BLACK|TB_BRIGHT);
                tb_set_cell(disp_x-1, disp_y, ' ', 0, TB_BLACK|TB_BRIGHT);
                tb_set_cell(disp_x+1, disp_y, ' ', 0, TB_BLACK|TB_BRIGHT);
                if(c.flags & CELL_FLAGGED) {
                    tb_set_cell(disp_x, disp_y, flag, 0, TB_BLACK|TB_BRIGHT);
                }
            }
        }
    }
    tb_present();
}


void draw_finished(GameData* g){
    uint16_t startx = display_grid_startx(g);
    uint16_t starty = display_grid_starty(g);

    for(int y=0; y < g->height; y++){
        for(int x=0; x < g->width; x++){
            CellData c = g->grid[y][x];

            uint16_t disp_x = startx+DISPLAY_GRID_X(x);
            uint16_t disp_y = starty+DISPLAY_GRID_Y(y);

            tb_set_cell(disp_x-1,disp_y,' ', 0, 0);
            tb_set_cell(disp_x+1,disp_y,' ', 0, 0);

            tb_set_cell(disp_x, disp_y, ' ' , 0, 0);
            if(c.flags & CELL_IS_MINE){
                tb_set_cell(disp_x,disp_y,mine, 0, 0);
            } else if(c.adjMines > 0){
                tb_printf(disp_x,disp_y,0,0,"%d", c.adjMines);
            }
        }
    }
    tb_present();
}

  

void draw_display_grid(GameData* g, uintattr_t fg, uintattr_t bg){
    uint16_t rows = DISPLAY_GRID_HEIGHT(g->height);
    uint16_t cols = DISPLAY_GRID_WIDTH(g->width);

    //int startx = (tb_width()-cols)/2;
    uint16_t startx = display_grid_startx(g);
    //uint16_t starty = (tb_height()-rows)/2;
    uint16_t starty = display_grid_starty(g);



    for(int y=0; y < rows; y++){
        for(int x =0; x < cols; x++){
            if((y % 2 == 0)  &&  (x % 4 == 0)){
                if(y == 0) { // top row
                    if(x == 0){
                        tb_set_cell(startx+x, starty+y, top_left, fg, bg);
                    } else if (x == cols-1){
                        tb_set_cell(startx+x, starty+y, top_right, fg, bg);
                    } else {
                        tb_set_cell(startx+x, starty+y, top_intersection, fg, bg);
                    }
                } else if(y == rows-1){ // bottom row
                    if(x == 0){
                        tb_set_cell(startx+x, starty+y, bottom_left, fg, bg);
                    } else if(x == cols-1){
                        tb_set_cell(startx+x, starty+y, bottom_right, fg, bg);
                    } else {
                        tb_set_cell(startx+x, starty+y, bottom_intersection, fg, bg);
                    }
                } else {
                    if(x == 0) {
                        tb_set_cell(startx+x, starty+y, intersection_left, fg, bg);
                    } else if(x == cols-1){
                        tb_set_cell(startx+x, starty+y, intersection_right, fg, bg);
                    } else {
                        tb_set_cell(startx+x, starty+y, intersection, fg, bg);
                    }
                } 
            } 
            if((y % 2 == 0) && (x % 4 != 0)){ // no intersections
                tb_set_cell(startx+x, starty+y, horizontal , fg, bg);
            } 
            if((y % 2 != 0) && (x % 4 == 0)){
                tb_set_cell(startx+x, starty+y, vertical, fg, bg);
            }
        }
    }
    tb_present();
}



// ----------------- Game Menu -----------------

uint8_t quit_dialogue(GameData* g){
    BoxCoordinates b;
    struct tb_event ev;

    b.width = DISPLAY_GRID_WIDTH(g->width) * 0.5;
    b.height = DISPLAY_GRID_HEIGHT(g->height) * 0.4;

    center_box(&b,tb_width(), tb_height());
    draw_box(b, TB_RED, 0);
    uint8_t sel = false;

    uintattr_t yes_color;
    uintattr_t no_color;

    clear_box_content(b);

    while(ev.key != TB_KEY_ENTER ){
        sel = (ev.ch == 'h' || ev.ch == 'l') ? !sel : sel;

        yes_color = (sel) ? TB_GREEN : 0;
        no_color = (!sel) ? TB_GREEN : 0;

        tb_printf(b.offset_x+1, b.offset_y, 0, 0, "quit game?");
        tb_printf(b.offset_x+1, b.offset_y+b.height-2, yes_color , 0, "[ yes ]" );
        tb_printf(b.offset_x+9, b.offset_y+b.height-2, no_color , 0, "[ no ]" );
        tb_present();

        tb_poll_event(&ev);
        clear_box_content(b);
    }

    return sel;
}

void make_game_selection(GameData* g){
    struct tb_event ev;
    int width = tb_width();
    int height = tb_height();

    BoxCoordinates menu_box;

    
    menu_box.width    = width*0.3;
    menu_box.height   = height*0.6;
    center_box(&menu_box, tb_width(), tb_height());

    draw_box(menu_box, TB_GREEN|TB_BOLD,0);

    GameSettings options[] = { // provisoric, should later be able to save custom game settings
        {8, 8, 10},   // Option 1: 8x8 grid, 10 mines
        {16, 16, 40}, // Option 2: 16x16 grid, 40 mines
        {30, 16, 99}  // Option 3: 24x24 grid, 99 mines
    };

    uint8_t number_of_options = 3; // including custom option
    uint8_t selection = 0;
    GameSettings selected_setting;
    // uint8_t number_of_options = sizeof(options) / sizeof(options[0]);

    while(true){
        clear_box_content(menu_box);
        draw_selection_menu(menu_box, selection, options, number_of_options);

        tb_poll_event(&ev);

        if(ev.key == TB_KEY_ARROW_DOWN || ev.ch == 'j'){
            if(selection >= number_of_options){
                selection = 0;
            } else {
                selection++;
            }
        }
        if(ev.key == TB_KEY_ARROW_UP || ev.ch == 'k'){
            if(selection <= 0){
                selection = number_of_options;
            } else {
                selection--;
            }
        }
        if(ev.key == TB_KEY_ENTER){
            if(selection == number_of_options){
                clear_box_content(menu_box);
                selected_setting = make_custom_selection(menu_box);
            } else {
                selected_setting = options[selection];
            }
            break;
        }
    }
    g->width = selected_setting.width;
    g->height = selected_setting.height;
    g->mine_count = selected_setting.mine_count;
    assert(g->width <= MAX_WIDTH && g->height <= MAX_HEIGHT);
}

GameSettings make_custom_selection(BoxCoordinates b){
    struct tb_event ev;
    uint8_t selection = 0;
    bool confirm = false;

    while(!confirm){
        clear_box_content(b);
        draw_custom_settings_menu(b, selection);

        tb_poll_event(&ev);

        if(ev.key == TB_KEY_ARROW_DOWN || ev.ch == 'j'){
            if(selection >= 2){
                selection = 0;
            } else {
                selection++;
            }
        }
        if(ev.key == TB_KEY_ARROW_UP || ev.ch == 'k'){
            if(selection <= 0){
                selection = 2;
            } else {
                selection--;
            }
        }
        if(ev.key == TB_KEY_ENTER){
            confirm = true;
        }
    }
}


void draw_custom_settings_menu(BoxCoordinates b, uint8_t selection){
    int offset_x = b.offset_x+(b.width-20) / 2;
    int offset_y = b.offset_y+(b.height-11)/2;

    BoxCoordinates width_input_box = {5, 3, offset_x+7, offset_y};
    BoxCoordinates height_input_box = {5, 3, offset_x+7, offset_y+4};
    BoxCoordinates mines_input_box = {5, 3, offset_x+7, offset_y+8};

    
    tb_printf(offset_x, offset_y+1, TB_BLUE, 0, "Width: " );
    if(selection == 0){
        draw_box(width_input_box,TB_BLUE,0);
    } else {
        draw_box(width_input_box,0,0);
    }


    tb_printf(offset_x-1, offset_y+5, 0, 0, "Height: " );
    if(selection == 1){
        draw_box(height_input_box,TB_BLUE,0);
    } else {
        draw_box(height_input_box,0,0);
    }
    tb_printf(offset_x, offset_y+9, 0, 0, "Mines: " );
    if(selection == 2){
        draw_box(mines_input_box,TB_BLUE,0);
    } else {
        draw_box(mines_input_box,0,0);

    }
}

void draw_selection_menu(BoxCoordinates b,uint8_t selection, GameSettings options[], uint8_t number_of_options){
    GameSettings curOption;
    int offset_x = b.offset_x + (b.width-18) / 2;
    int offset_y = b.offset_y+  (b.height-8)/2;

    tb_printf(offset_x, offset_y, TB_BOLD+TB_MAGENTA, 0, "select difficulty:");
    offset_y += 3;


    if(selection == number_of_options){
        tb_set_cell(offset_x-1, offset_y+number_of_options+1, '>', TB_BLUE, 0);
        tb_printf(offset_x, offset_y+number_of_options+1,TB_BLUE,0, "custom");
    } else {
        tb_printf(offset_x, offset_y+number_of_options+1,0,0, "custom");
    }

    for(int i = 0; i < number_of_options; i++){
        curOption = options[i];
        if(i == selection){
            tb_printf(offset_x, offset_y+i,TB_BLUE,0, "%d x %d (%d Mines)", curOption.width, curOption.height, curOption.mine_count);
            tb_set_cell(offset_x-1, offset_y+i, '>', TB_BLUE, 0);
        }else{
            tb_printf(offset_x, offset_y+i,0,0, "%d x %d (%d Mines)", curOption.width, curOption.height, curOption.mine_count);
        }
    }
    tb_present();
}

void clear_box_content(BoxCoordinates b){
    assert(b.width + b.offset_x < tb_width() && "Box width must be smaller than terminal width");
    assert(b.height + b.offset_y < tb_height() && "Box height must be smaller than terminal height");

    for(int y = 1; y < b.height-1; y++){
        for(int x = 1; x < b.width-1; x++){
            tb_set_cell(b.offset_x+x, b.offset_y+y, ' ' ,0, 0);
        }
    }
    tb_present();
}

void center_box(BoxCoordinates* b, int rel_width, int rel_height){
    b->offset_x = (rel_width - b->width)/2;
    b->offset_y = (rel_height - b->height)/2;
}

void draw_box(BoxCoordinates b, uintattr_t fg, uintattr_t bg){
   assert(b.width + b.offset_x < tb_width() && "Box width (including padding) must be smaller than terminal width");
   assert(b.height + b.offset_y < tb_height() && "Box height (including padding) must be smaller than terminal height");

    for(int y = 0; y < b.height; y++){
        for(int x = 0; x < b.width; x++){
            //corner edge cases
            if(x == 0 && y == 0){ 
                tb_set_cell(b.offset_x + x ,b.offset_y+y, top_left_round, fg,bg);
            } else if (x == b.width-1 && y == 0){ // top right corner
                tb_set_cell(b.offset_x + x ,b.offset_y+y, top_right_round,fg,bg);
            } else if (x == 0 && y == b.height-1) { // bot left corner
                tb_set_cell(b.offset_x + x ,b.offset_y+y, bottom_left_round, fg,bg);
            } else if (x == b.width-1 && y == b.height-1){ // bottom right corner
                tb_set_cell(b.offset_x + x ,b.offset_y+y, bottom_right_round, fg,bg);
            }

            else if (x == 0 || x == b.width-1) { // vertical edges
                tb_set_cell(b.offset_x + x ,b.offset_y+y, vertical, fg,bg);
            } else if (y == 0 || y == b.height-1) {
                tb_set_cell(b.offset_x + x ,b.offset_y+y, horizontal, fg,bg);
            }
        }
    }
    tb_present();
}


