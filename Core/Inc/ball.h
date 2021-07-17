#ifndef INC_BALL_H_
#define INC_BALL_H_
#include "stm32f429i_discovery_gyroscope.h"
#include "stm32f429i_discovery_lcd.h"
#include <stdio.h>

#define RADIUS 20

struct Ball
{
	int   x;
	int   y;
	int   color;
	int   r;
	struct Ball *eraser;
};

void Draw_Ball(struct Ball* b);
void Init_Ball(struct Ball* b, int x_, int y_, int color_, struct Ball* eraser_);
void Erase_Ball(struct Ball* b);
void Update_Ball(struct Ball* b);
#endif /* INC_BALL_H_ */
