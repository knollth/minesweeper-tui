#define TB_IMPL

#include "../include/termbox2.h"
#include <wchar.h>
#include <locale.h>
#include <stdbool.h>

// Box-drawing Unicode characters (using Nerd Fonts)
uint32_t top_left = 0x256D;   // '╭'
uint32_t top_right = 0x256E;  // '╮'
uint32_t bottom_left = 0x2570; // '╰'
uint32_t bottom_right = 0x256F; // '╯'
uint32_t horizontal = 0x2500;  // '─'
uint32_t vertical = 0x2502;    // '│'


void draw_selection();

int main() {
    struct tb_event ev;
    int y = 0;

    tb_init();
	setlocale(LC_ALL, ""); /* Fix unicode handling */


    tb_printf(0, y++, TB_GREEN, 0, "hello from termbox");
    tb_printf(0, y++, 0, 0, "width=%d height=%d", tb_width(), tb_height());
    tb_printf(0, y++, 0, 0, "press any key...");
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
    draw_selection();
    //tb_present();


    tb_poll_event(&ev);


    tb_shutdown();

    return 0;
}

void draw_selection(){
    int width = tb_width();
    int height = tb_height();

    int box_width = width*0.3;
    int box_height = height*0.6;

    int padding_x = (width - box_width)/2;
    int padding_y = (height - box_height)/2;

    int x,y;

    for(y = 0; y < box_height; y++){
        for(x = 0; x < box_width; x++){
            //corner edge cases
            if(x == 0 && y == 0){ 
                tb_set_cell_ex(padding_x + x ,padding_y+y, &top_left, wcwidth(top_left), TB_WHITE,0);
            } else if (x == box_width-1 && y == 0){ // top right corner
                tb_set_cell_ex(padding_x + x ,padding_y+y, &top_right, wcwidth(top_right),TB_WHITE,0);
            } else if (x == 0 && y == box_height-1) { // bot left corner
                tb_set_cell_ex(padding_x + x ,padding_y+y, &bottom_left, wcwidth(bottom_left), TB_WHITE,0);
            } else if (x == box_width-1 && y == box_height-1){ // bottom right corner
                tb_set_cell_ex(padding_x + x ,padding_y+y, &bottom_right, wcwidth(bottom_right), TB_WHITE,0);
            }

            else if (x == 0 || x == box_width-1) { // vertical edges
                tb_set_cell_ex(padding_x + x ,padding_y+y, &vertical, wcwidth(vertical), TB_WHITE,0);
            } else if (y == 0 || y == box_height-1) {
                tb_set_cell_ex(padding_x + x ,padding_y+y, &horizontal , wcwidth(horizontal), TB_WHITE,0);
            }
        }
    }
    tb_present();
    struct tb_event ev;
    bool confirm = false;
    int text_y_padding = padding_y+(box_height/2) - 5;

    tb_set_cell(padding_x+2, text_y_padding+6, '>', 0,0);

    do{
        text_y_padding = padding_y+(box_height/2) - 5;
        tb_printf(padding_x+2, text_y_padding++ , 0, 0, "select difficulty");
        text_y_padding +=5;
        tb_printf(padding_x+2, text_y_padding++ , 0, 0, "8x8");
        tb_printf(padding_x+2, text_y_padding++, 0, 0, "16x16");
        tb_printf(padding_x+2, text_y_padding++, 0, 0, "custom");
        tb_present();
        tb_poll_event(&ev);
        if(ev.key == 0x0d){
            confirm = true;
        }
    } while (!confirm);
}


