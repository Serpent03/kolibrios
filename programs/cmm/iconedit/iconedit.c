/*
 * Icon Editor for KolibriOS
 * Author: Leency
 * Licence: GPL v2
*/

/*
TODO:
arange tools
window colors
enhance icon
*/

#define MEMSIZE 4096*40

#include "../lib/gui.h"
#include "../lib/random.h"
#include "../lib/mem.h"
#include "../lib/obj/libimg.h"
#include "../lib/patterns/rgb.h"
#include "../lib/patterns/libimg_load_skin.h"

#include "colors_mas.h"

//===================================================//
//                                                   //
//                       DATA                        //
//                                                   //
//===================================================//

#define T_TITLE "Icon Editor 0.15"

#define TOOLBAR_H    24+8
#define PALLETE_SIZE 116

rect wrapper = { 10, TOOLBAR_H, NULL, NULL };
rect right_bar = { NULL, TOOLBAR_H, 280, NULL };
rect canvas = { NULL, NULL, NULL, NULL };

dword active_color_1 = 0x000000;
dword active_color_2 = 0xFFFfff;

enum {
	BTN_NEW = 40,
	BTN_OPEN,
	BTN_SAVE,
	BTN_MOVE_LEFT,
	BTN_MOVE_RIGHT,
	BTN_MOVE_UP,
	BTN_MOVE_DOWN,
	BTN_FLIP_HOR,
	BTN_FLIP_VER,
	BTN_ROTATE_LEFT,
	BTN_ROTATE_RIGHT,
	BTN_PICK,
	BTN_FILL,
	BTN_ZOOM_IN,
	BTN_ZOOM_OUT,
	BTNS_PALETTE_COLOR_MAS = 100,
	BTNS_LAST_USED_COLORS = 400
};

proc_info Form;

more_less_box zoom = { NULL, NULL, 11, 1, 40, BTN_ZOOM_IN, BTN_ZOOM_OUT, "Zoom" };

dword default_palette[] = {
0x330000,0x331900,0x333300,0x193300,0x003300,0x003319,0x003333,0x001933,0x000033,0x190033,0x330033,0x330019,0x000000,
0x660000,0x663300,0x666600,0x336600,0x006600,0x006633,0x006666,0x003366,0x000066,0x330066,0x660066,0x660033,0x202020,
0x990000,0x994C00,0x999900,0x4C9900,0x009900,0x00994C,0x009999,0x004C99,0x000099,0x4C0099,0x990099,0x99004C,0x404040,
0xCC0000,0xCC6600,0xCCCC00,0x66CC00,0x00CC00,0x00CC66,0x00CCCC,0x0066CC,0x0000CC,0x6600CC,0xCC00CC,0xCC0066,0x606060,
0xFF0000,0xFF8000,0xFFFF00,0x80FF00,0x00FF00,0x00FF80,0x00FFFF,0x0080FF,0x0000FF,0x7F00FF,0xFF00FF,0xFF007F,0x808080,
0xFF3333,0xFF9933,0xFFFF33,0x99FF33,0x33FF33,0x33FF99,0x33FFFF,0x3399FF,0x3333FF,0x9933FF,0xFF33FF,0xFF3399,0xA0A0A0,
0xFF6666,0xFFB266,0xFFFF66,0xB2FF66,0x66FF66,0x66FFB2,0x66FFFF,0x66B2FF,0x6666FF,0xB266FF,0xFF66FF,0xFF66B2,0xC0C0C0,
0xFF9999,0xFFCC99,0xFFFF99,0xCCFF99,0x99FF99,0x99FFCC,0x99FFFF,0x99CCFF,0x9999FF,0xCC99FF,0xFF99FF,0xFF99CC,0xE0E0E0,
0xFFCCCC,0xFFE5CC,0xFFFFCC,0xE5FFCC,0xCCFFCC,0xCCFFE5,0xCCFFFF,0xCCE5FF,0xCCCCFF,0xE5CCFF,0xFFCCFF,0xFFCCE5,0xFFFFFF	
};
dword last_used_colors[13*2] = {
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,
0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF,0xFFFFFF
};

_image image;

libimg_image open_image;

enum {
	PENCIL,
	FILL,
	PIPET
};
int active_tool = PENCIL;

//===================================================//
//                                                   //
//                       CODE                        //
//                                                   //
//===================================================//

void main()
{
	word btn;

	load_dll(libio,  #libio_init,  1);
	load_dll(libimg, #libimg_init, 1);
	Libimg_LoadImage(#skin, "/sys/icons16.png");
	//system.color.get();
	//Libimg_ReplaceColor(skin.image, skin.w, skin.h, 0xFFfffFFF, system.color.work_text);

	image.create(32, 32);

	if (param[0]) {
		Libimg_LoadImage(#open_image, #param);
		if (open_image.w==32) && (open_image.h==32) {
			image.set_image(open_image.imgsrc);
		}
		else {
			notify("'Error: image format is unacceptable.\nOnly images created in IconEditor (BMP, 32x32x16b) can be opened!' -E");
		}
	}

	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);

	loop() switch(WaitEvent())
	{
		case evMouse:
			mouse.get();
			if (mouse.vert) {
				if (mouse.vert==65535) zoom.click(BTN_ZOOM_IN);
				if (mouse.vert==1) zoom.click(BTN_ZOOM_OUT);
				DrawEditArea();
			}
			if (active_tool == PIPET)
			{
				 EventPickColor(mouse.lkm, mouse.pkm);
			}
			if (active_tool == PENCIL) {
				if (mouse.x>canvas.x) && (mouse.y>canvas.y) 
				&& (mouse.y<canvas.y+canvas.h) && (mouse.x<canvas.x+canvas.w)
				{
					if (mouse.lkm) image.set_pixel(mouse.y-canvas.y/zoom.value, 
						mouse.x-canvas.x/zoom.value, active_color_1);
					if (mouse.pkm) image.set_pixel(mouse.y-canvas.y/zoom.value, 
						mouse.x-canvas.x/zoom.value, active_color_2);
					DrawCanvas();
				}
			}
			if (active_tool == FILL) {
				if (mouse.x>canvas.x) && (mouse.y>canvas.y) 
				&& (mouse.y<canvas.y+canvas.h) && (mouse.x<canvas.x+canvas.w)
				{
					if (mouse.lkm) EventFill(mouse.y-canvas.y/zoom.value, 
						mouse.x-canvas.x/zoom.value, active_color_1);
					if (mouse.pkm) EventFill(mouse.y-canvas.y/zoom.value, 
						mouse.x-canvas.x/zoom.value, active_color_2);
					DrawCanvas();
				}
			}
			break;

		case evButton:
			btn = GetButtonID();
			switch(btn)
			{
				case BTN_NEW:
					image.create(32, 32);
					DrawCanvas();
					break;
				case BTN_OPEN:
					RunProgram("/sys/lod", sprintf(#param, "*bmp* %s",#program_path));
					break;
				case BTN_SAVE:
					EventSave();
					break;
				case BTN_MOVE_LEFT:
					image.move(MOVE_LEFT);
					DrawCanvas();
					break;
				case BTN_MOVE_RIGHT:
					image.move(MOVE_RIGHT);
					DrawCanvas();
					break;
				case BTN_MOVE_UP:
					image.move(MOVE_UP);
					DrawCanvas();
					break;
				case BTN_MOVE_DOWN:
					image.move(MOVE_DOWN);
					DrawCanvas();
					break;
				case BTN_FLIP_VER:
					image.move(FLIP_VER);
					DrawCanvas();
					break;
				case BTN_FLIP_HOR:
					image.move(FLIP_HOR);
					DrawCanvas();
					break;
				case BTN_PICK:
					EventPickActivate();
					break;
				case BTN_FILL:
					EventFillActivate();
					break;
				case BTN_ZOOM_IN:
				case BTN_ZOOM_OUT:
					zoom.click(btn);
					DrawEditArea();
					break;
				case CLOSE_BTN:
					ExitProcess();
					break;
			}              
			if (btn >= BTNS_PALETTE_COLOR_MAS) && (btn < BTNS_PALETTE_COLOR_MAS+PALLETE_SIZE) 
			{ 
				if (mouse.lkm) EventSetActiveColor(1, default_palette[btn - BTNS_PALETTE_COLOR_MAS]);
				if (mouse.pkm) EventSetActiveColor(2, default_palette[btn - BTNS_PALETTE_COLOR_MAS]);
			}
			if (btn >= BTNS_LAST_USED_COLORS) && (btn < sizeof(last_used_colors)*sizeof(dword))
			{
				if (mouse.lkm) active_color_1 = last_used_colors[btn - BTNS_LAST_USED_COLORS]; 
				if (mouse.pkm) active_color_2 = last_used_colors[btn - BTNS_LAST_USED_COLORS];
				DrawActiveColor(NULL);
			}
			break;
	  
		case evKey:
			GetKeys();
			if (key_scancode == SCAN_CODE_ESC) EventPickDeactivate();
			if (key_scancode == SCAN_CODE_KEY_I) EventPickActivate();
			//if (key_scancode == SCAN_CODE_KEY_F) EventFillActivate();
			if (key_scancode == SCAN_CODE_MINUS) {zoom.click(BTN_ZOOM_OUT); DrawEditArea();}
			if (key_scancode == SCAN_CODE_PLUS)  {zoom.click(BTN_ZOOM_IN);  DrawEditArea();}
			break;
		 
		case evReDraw:
			draw_window();
			break;
	}
}

void DrawToolbarButton(dword _id, _x, _icon_n)
{
	DrawWideRectangle(_x, 4, 22, 22, 3, 0xFFFfff);
	DefineHiddenButton(_x, 4, 21, 21, _id);
	img_draw stdcall(skin.image, _x+3, 7, 16, 16, 0, _icon_n*16);
}

void DrawStatusBar()
{
	zoom.y = wrapper.y + wrapper.h + 6;
	zoom.x = wrapper.x;
	zoom.draw();

	sprintf(#param,"Canvas: %ix%i", image.rows, image.columns);
	WriteText(wrapper.x+wrapper.w-calc(strlen(#param)*8), zoom.y+2, 0x90, system.color.work_text, #param);
}

void draw_window()
{
	#define TB_ICON_PADDING 26
	incn tx;
	system.color.get();
	DefineAndDrawWindow(115+random(100), 50+random(100), 700, 540, 0x33, system.color.work, T_TITLE, 0);
	GetProcessInfo(#Form, SelfInfo);
	if (Form.status_window>2) return;
	if (Form.width  < 560) { MoveSize(OLD,OLD,560,OLD); return; }
	if (Form.height < 430) { MoveSize(OLD,OLD,OLD,430); return; }

	right_bar.x = Form.cwidth - right_bar.w;

	tx.n = 10-TB_ICON_PADDING;
	DrawToolbarButton(BTN_NEW,    tx.inc(TB_ICON_PADDING), 2); //not implemented
	DrawToolbarButton(BTN_OPEN,   tx.inc(TB_ICON_PADDING), 0); //not implemented
	DrawToolbarButton(BTN_SAVE,   tx.inc(TB_ICON_PADDING), 5);
	DrawToolbarButton(BTN_MOVE_LEFT,  tx.inc(TB_ICON_PADDING+8),   30);
	DrawToolbarButton(BTN_MOVE_RIGHT, tx.inc(TB_ICON_PADDING),   31);
	DrawToolbarButton(BTN_MOVE_UP,    tx.inc(TB_ICON_PADDING),   32);
	DrawToolbarButton(BTN_MOVE_DOWN,  tx.inc(TB_ICON_PADDING),   33);
	
	DrawToolbarButton(BTN_FLIP_HOR,   tx.inc(TB_ICON_PADDING+8), 34);
	DrawToolbarButton(BTN_FLIP_VER,   tx.inc(TB_ICON_PADDING),   35);
	// DrawToolbarButton(BTN_ROTATE_LEFT,   tx.inc(TB_ICON_PADDING), 36); //not implemented
	// DrawToolbarButton(BTN_ROTATE_RIGHT,  tx.inc(TB_ICON_PADDING), 37); //not implemented

	DrawToolbarButton(BTN_PICK,   tx.inc(TB_ICON_PADDING+8), 38);
	DrawToolbarButton(BTN_FILL,   tx.inc(TB_ICON_PADDING), 39);

	DrawEditArea();

	DrawActiveColor(right_bar.y);
	DrawColorPallets(right_bar.x, right_bar.y + 30);

	DrawStatusBar();
}

void DrawEditArea()
{
	dword color1=0xC0C0C0;
	int top_side;
	int left_side;

	wrapper.w = Form.cwidth - right_bar.w - 30;
	wrapper.h = Form.cheight - TOOLBAR_H - 35;

	//canvas{
	canvas.w = image.columns * zoom.value;
	canvas.h = image.rows * zoom.value;
	if (canvas.w+2 > wrapper.w) || (canvas.h+2 > wrapper.h) { 
		zoom.click(BTN_ZOOM_OUT);
		DrawEditArea();
		return;
	}
	canvas.x = -zoom.value*image.columns+wrapper.w/2 + wrapper.x;
	canvas.y = -zoom.value*image.rows+wrapper.h/2 + wrapper.y;
	DrawRectangle(canvas.x-1, canvas.y-1, canvas.w+1, canvas.h+1, 0x808080);
	DrawCanvas();
	//}

	left_side = canvas.x-wrapper.x-1;
	top_side = canvas.y-wrapper.y-1;

	DrawRectangle(wrapper.x-1, wrapper.y-1, wrapper.w, wrapper.h, system.color.work_graph);

	if (left_side>0)
	{
		DrawBar(wrapper.x, wrapper.y, wrapper.w-1, top_side, color1); //top
		DrawBar(wrapper.x, wrapper.y+wrapper.h-top_side-1, wrapper.w-1, top_side, color1); //bottom
	}
	if (top_side>0)
	{
		DrawBar(wrapper.x, wrapper.y+top_side, left_side, wrapper.h-top_side-top_side, color1); //left
		DrawBar(wrapper.x+wrapper.w-left_side-1, wrapper.y+top_side, left_side, wrapper.h-top_side-top_side, color1); //right
	}
}

void DrawActiveColor(dword iny)
{
	static dword outy;
	if (iny != NULL) outy = iny;
	DrawBar(right_bar.x, outy, 20, 20, active_color_1);
	sprintf(#param, "%A", active_color_1);
	EDI = system.color.work;
	WriteText(right_bar.x + 30, outy + 3, 0xD0, system.color.work_text, #param+4);

	DrawBar(right_bar.x+110, outy, 20, 20, active_color_2);
	sprintf(#param, "%A", active_color_2);
	EDI = system.color.work;
	WriteText(right_bar.x+110 + 30, outy + 3, 0xD0, system.color.work_text, #param+4);	
}

void DrawColorPallets(dword _x, _y)
{
	int r, c, i=0;
	int cellw = 20;

	//Last used colors
	for (r = 0; r < 2; r++)
	{
		for (c = 0; c < 13; c++, i++)
		{
			DrawBar(c*cellw + _x, r*cellw + _y, cellw, cellw, last_used_colors[i]);
			DefineHiddenButton(c*cellw + _x, r*cellw + _y, cellw-1, cellw-1, BTNS_LAST_USED_COLORS+i);
			
		}
	}

	_y += r*cellw + 10;
	i=0;

	//Default colors
	for (r = 0; r < 9; r++)
	{
		for (c = 0; c < 13; c++, i++)
		{
			DrawBar(c*cellw + _x, r*cellw + _y, cellw, cellw, default_palette[PALLETE_SIZE-i]);
			DefineHiddenButton(c*cellw + _x, r*cellw + _y, cellw-1, cellw-1, BTNS_PALETTE_COLOR_MAS+PALLETE_SIZE-i);
		}
	}
}


void DrawCanvas()
{
	int r, c;
	for (r = 0; r < image.rows; r++)
	{
		for (c = 0; c < image.columns; c++)
		{
			DrawBar(c*zoom.value + canvas.x, r*zoom.value + canvas.y, 
				zoom.value, zoom.value, image.get_pixel(r, c));
		}
	}
}


//===================================================//
//                                                   //
//                      EVENTS                       //
//                                                   //
//===================================================//

void EventPickActivate()
{
	SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE);
	active_tool = PIPET;
}

void EventPickDeactivate()
{
	active_tool = PENCIL;
}

void EventFillActivate()
{
	active_tool = FILL;
}

void EventPickColor(dword lkm_status, pkm_status)
{
	active_color_1 = GetPixelColorFromScreen(mouse.x + Form.left + 5, mouse.y + Form.top + skin_height);
	DrawActiveColor(NULL);
	if (mouse.down) && (mouse.key&MOUSE_LEFT) {
		EventPickDeactivate();
		SetEventMask(EVM_REDRAW+EVM_KEY+EVM_BUTTON+EVM_MOUSE+EVM_MOUSE_FILTER);
		EventSetActiveColor(1, active_color_1);
	}
}

dword bmp_32x32x16_header[] = FROM "bmp32x32header";
void EventSave()
{
	char save_buf[3126];
	memmov(#save_buf, #bmp_32x32x16_header, sizeof(bmp_32x32x16_header));
	memmov(#save_buf+sizeof(bmp_32x32x16_header), image.get_image(), sizeof(save_buf)-sizeof(bmp_32x32x16_header));
	if (WriteFile(sizeof(save_buf), #save_buf, "/rd/1/saved_image.bmp")==0)
	{
		notify("'File saved as /rd/1/saved_image.bmp' -O");
	}
	else {
		notify("'Error saving BPM file, probably not enought space on ramdisk!' -E");
	}
}

void EventSetActiveColor(int _number, _color)
{
	int i;
	for (i=13*2-1; i>0; i--) {
		last_used_colors[i] = last_used_colors[i-1];
	}
	last_used_colors[0] = _color;

	if (_number == 1) active_color_1 = _color;
	if (_number == 2) active_color_2 = _color;

	DrawActiveColor(NULL);
	DrawColorPallets(right_bar.x, right_bar.y + 30);
}

void EventFill(dword _r, _c, _color)
{
	#define MARKED 6
	int r, c, i, restart;

	dword old_color = image.get_pixel(_r, _c);
	image.set_pixel(_r, _c, MARKED);

	do {
		restart=false;	
		for (r = 0; r < image.rows; r++)
			for (c = 0; c < image.columns; c++)
			{
				IF (image.get_pixel(r,c) != old_color) continue;
				IF (image.get_pixel(r,c) == MARKED) continue;
				
				IF (c>0)               && (image.get_pixel(r,c-1) == MARKED) restart=true;
				IF (c<image.columns-1) && (image.get_pixel(r,c+1) == MARKED) restart=true;
				IF (r>0)               && (image.get_pixel(r-1,c) == MARKED) restart=true;
				IF (r<image.rows-1)    && (image.get_pixel(r+1,c) == MARKED) restart=true;
				
				IF (restart == true) image.set_pixel(r,c,MARKED);
			}
	}while(restart);

	for (i=0; i<image.columns*image.rows; i++) 
			IF (image.mas[i]==MARKED) image.mas[i] = _color;
}
