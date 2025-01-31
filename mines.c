#include "mines.h"
#include <locale.h>
#include <stdbool.h>
#include <assert.h>
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

int main() {
    struct tb_event ev;
    int y = 0;

    tb_init();
    setlocale(LC_ALL, ""); /* Fix unicode handling */
    /*
    tb_printf(0, y++, TB_GREEN, 0, "hello from termbox");
    tb_printf(0, y++, 0, 0, "width=%d height=%d", tb_width(), tb_height());
    tb_printf(0, y++, 0, 0, "press any key...");
    tb_printf(0, y++, 0, 0, "top_left_round width: %d", wcwidth(top_left_round));
    tb_printf(0, y++, 0, 0, "top_left width: %d", wcwidth(top_left));
    tb_printf(0, y++, 0, 0, "intersection width: %d", wcwidth(intersection));
    tb_printf(0, y++, 0, 0, "left_intersection width: %d", wcwidth(intersection_left));
    tb_present();

    tb_poll_event(&ev);

    y++;
    tb_printf(0, y++, 0, 0, "event type=%d key=%d ch=%c", ev.type, ev.key, ev.ch);
    tb_printf(8, y++, 0, 0, "width of character ╭ : %d", top_left, wcwidth(top_left));
    tb_present();

    tb_poll_event(&ev);
    
    y++;
    tb_set_cell(4, y++, 'H', TB_BLUE,0);
    tb_set_cell_ex(4, y++, &top_left,wcwidth(top_left), TB_BLUE,0);
    tb_set_cursor(4, 4);
    tb_present();
    tb_clear();
    tb_hide_cursor();
    */
    //tb_present();
    init_game();


    //tb_poll_event(&ev);


    tb_shutdown();

    return 0;
}

void init_game(){
    struct tb_event ev;
    GameSettings settings = make_game_selection();
    tb_clear();
    tb_printf(0, 0, 0, 0, "Height: %d, Width: %d, Mines: %d", settings.height, settings.width, settings.mine_count);
    tb_present();
    tb_poll_event(&ev);
    tb_clear();
    draw_grid(0, 0, settings.width, settings.height, 0, 0);
    tb_present();
    tb_poll_event(&ev);
}

GameSettings make_game_selection(){
    struct tb_event ev;
    uint8_t width = tb_width();
    uint8_t height = tb_height();

    BoxCoordinates menu_box;

    
    menu_box.width    = width*0.3;
    menu_box.height   = height*0.6;
    menu_box.offset_x = (width - menu_box.width)/2;
    menu_box.offset_y = (height - menu_box.height)/2;
    
    draw_box(menu_box, TB_GREEN|TB_BOLD,0);

    uint8_t selection = 0;
    

    GameSettings options[] = { // provisoric, should later be able to save custom game settings
        {8, 8, 10},   // Option 1: 8x8 grid, 10 mines
        {16, 16, 40}, // Option 2: 16x16 grid, 40 mines
        {24, 24, 99}  // Option 3: 24x24 grid, 99 mines
    };
    uint8_t number_of_options = 3; // including custom option
    // uint8_t number_of_options = sizeof(options) / sizeof(options[0]);

    while(true){
        clear_box_content(menu_box);
        draw_selection_menu(menu_box, selection ,options, number_of_options);

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
                make_custom_selection(menu_box);
            } else {
                break;
            }
        }
    }
    return options[selection];
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
    uint8_t offset_x = b.offset_x+(b.width-20) / 2;
    uint8_t offset_y = b.offset_y+(b.height-11)/2;

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
    uint8_t offset_x = b.offset_x + (b.width-18) / 2;
    uint8_t offset_y = b.offset_y+  (b.height-8)/2;

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

void draw_grid(uint8_t startx, uint8_t starty, uint8_t cellcols, uint8_t cellrows, uintattr_t fg, uintattr_t bg){
    uint16_t rows = cellrows*2+1;
    uint16_t cols = cellcols*4+1;

    for(int y=0; y < rows; y++){
        for(int x =0; x < cols; x++){
            if((y % 2 == 0)  &&  (x % 4 == 0)){
                if(y == 0) { // top row
                    if(x == 0){
                        tb_set_cell(x, y, top_left, fg, bg);
                    } else if (x == cols-1){
                        tb_set_cell(x, y, top_right, fg, bg);
                    } else {
                        tb_set_cell(x, y, top_intersection, fg, bg);
                    }
                } else if(y == rows-1){ // bottom row
                    if(x == 0){
                        tb_set_cell(x, y, bottom_left, fg, bg);
                    } else if(x == cols-1){
                        tb_set_cell(x, y, bottom_right, fg, bg);
                    } else {
                        tb_set_cell(x, y, bottom_intersection, fg, bg);
                    }
                } else {
                    if(x == 0) {
                        tb_set_cell(x, y, intersection_left, fg, bg);
                    } else if(x == cols-1){
                        tb_set_cell(x, y, intersection_right, fg, bg);
                    } else {
                        tb_set_cell(x, y, intersection, fg, bg);
                    }
                } 
            } 
            if((y % 2 == 0) && (x % 4 != 0)){ // no intersections
                tb_set_cell(x, y, horizontal , fg, bg);
            } 
            if((y % 2 != 0) && (x % 4 == 0)){
                tb_set_cell(x, y, vertical, fg, bg);
            }
            
        }
    }
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


