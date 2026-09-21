/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

#include <hardware.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;

// Jumps a line or if the screen limit is reached it shifts the buffer and clears the final line
void line_jump_cursor() {
	x = 0;
	if (y + 1 >= NUM_ROWS) {
		Word *screen = (Word *)0xb8000;
		for (int i = 1; i < NUM_ROWS; i++) {
			for (int j = 0; j < NUM_COLUMNS; j++) {
				screen[((i - 1) * NUM_COLUMNS + j)] = screen[(i * NUM_COLUMNS + j)];
			}
		}
		for (int j = 0; j < NUM_COLUMNS; j++) screen[(y * NUM_COLUMNS + j)] = 0;
	} else y++;
}

void printc(char c)
{
  bochs_out(c);
  if (c=='\n')
  {
		line_jump_cursor();
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
			line_jump_cursor();
    }
  }
}

void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}
