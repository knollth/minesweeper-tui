#include "mines.h"
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
#include <stdlib.h>
#include <wchar.h>



// Box-drawing Unicode characters (using Nerd Fonts)
uint32_t top_left_round = 0x256D;   // '╭'
uint32_t top_right_round = 0x256E;  // '╮'
uint32_t bottom_left_round = 0x2570; // '╰'
uint32_t bottom_right_round = 0x256F; // '╯'
uint32_t horizontal = 0x2500;  // '─'
uint32_t vertical = 0x2502;    // '│'
uint32_t vert_b = 0x2503;    // '┃'
//
uint32_t top_left = 0x250C;    // '┌'
uint32_t top_right = 0x2510;   // '┐'
uint32_t bottom_left = 0x2514; // '└'
uint32_t bottom_right = 0x2518; // '┘'
uint32_t intersection = 0x253C; // '┼'
uint32_t top_intersection = 0x252C; // '┬'
uint32_t bottom_intersection = 0x2534; // '┴'
uint32_t intersection_left = 0x251C; // '├'
uint32_t intersection_right = 0x2524; // '┤'
//
wchar_t flag = L'⚑';
wchar_t mine = L'💣';



int main() {
    //struct tb_event ev;
    //int y = 0;

    tb_init();
    setlocale(LC_ALL, ""); /* Fix unicode handling */
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
        tb_poll_event(&ev);
        return;
    }

    tb_clear();
    tb_printf(0, 0, 0, 0, "Height: %d, Width: %d, Mines: %d",
              g.width, g.height, g.mine_count);
    tb_present();
    tb_poll_event(&ev);
    tb_clear();



    int grid_startx = get_center_x_offset(get_termgrid_width(g.width));
    int grid_starty = get_center_y_offset(get_termgrid_height(g.height));
    draw_grid(grid_startx, grid_starty, g.width, g.height, 0, 0);
    place_mines(&g);
    draw_mines(grid_startx, grid_starty, &g);

    tb_poll_event(&ev);
    free_game_grid(&g);
    return;
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
void place_mines(GameData* g){
    int n = g->width * g->height;
    CellData* curCel;
    uint16_t x;
    uint16_t y;

    srand(time(NULL));

    for(int count = 0; count < g->mine_count;){
        int i = rand() % n;
        x = i%(g->width);
        y = i/(g->width);

        curCel = &g->grid[y][x];

        if(!(curCel->isMine)){
            curCel->isMine = true;
            count++;
        }
    }
}

void get_adjacent_mines(uint x, int y, GameData* g){
    assert("cell must be within game grid"
           && y >= 0 && y < g->height 
           && x >= 0 && x < g->width);



}
// ----------------- Game Rendering -----------------

void draw_mines(int startx, int starty, GameData* g){
    for(int y=0; y < g->height; y++){
        for(int x=0; x < g->width; x++){
            if(g->grid[y][x].isMine){
                tb_set_cell(startx+get_termgrid_x(x),
                            starty+get_termgrid_y(y),
                            mine, 0, 0);
            }
        }
    }
    tb_present();
}
int get_termgrid_x(int x){
    return 2+(4*x);
}

int get_termgrid_y(int y){
    return 1+(2*y);
}

int get_termgrid_height(int height){
    return height* 2+1;
}
int get_termgrid_width(int width){
    return width*4+1;
}

void draw_grid(int startx, int starty, int width, int height, uintattr_t fg, uintattr_t bg){
    int rows = height*2+1;
    int cols = width*4+1;

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


void make_game_selection(GameData* g){
    struct tb_event ev;
    int width = tb_width();
    int height = tb_height();

    BoxCoordinates menu_box;

    
    menu_box.width    = width*0.3;
    menu_box.height   = height*0.6;
    //menu_box.offset_x = (width - menu_box.width)/2;
    //menu_box.offset_y = (height - menu_box.height)/2;
    menu_box.offset_x = get_center_x_offset(menu_box.width);
    menu_box.offset_y = get_center_y_offset(menu_box.height);
    
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

    for(int y = 1; y < b.height-2; y++){
        for(int x = 1; x < b.width-2; x++){
            tb_set_cell(b.offset_x+x, b.offset_y+y, ' ' ,0, 0);
        }
    }
    tb_present();
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


int get_center_x_offset(int width){
    return (tb_width() - width)/2;
}
int get_center_y_offset (int height){
    return (tb_height() - height)/2;
}


