#include "ball.h"
#include <stdlib.h>

int xData_act  = 0, yData_act  = 0;
int xData_prev = 0, yData_prev = 0;
int xDiff      = 0, yDiff      = 0;
int xVel       = 0, yVel       = 0; // velocity on axes
int xSum_vel   = 0, ySum_vel   = 0; // summary velocity on axes
int xAdd       = 0, yAdd       = 0; // additional component of summary

uint32_t tick_decrease = 0; // stores ticks for additional component decrease
uint32_t current_tick  = 0;  // stores ticks for getting axes data

extern float             xError_average;
extern float             yError_average;
extern TIM_HandleTypeDef htim6;

char xBuf[64];
char yBuf[64];

float Axes_Data[3] = {0,0,0};

static inline int Translate(float tr_data) { return (int)(tr_data/1000); }
static inline int Downgrade(int value) { return (int)(value/10); }

/*
 * Checking new ball coordinates
 * x, r -- new coordinates calculated in update function
 *
 */
void Bodred_Check(int x, int y, int r, struct Ball* b)
{
	//x-borders check:
	if( (x - r > 0)  && (x + r < ILI9341_LCD_PIXEL_WIDTH - 1) ) //inside border
		b->x = x;
	else if(x - r <= 0) //left border
	{
		b->x = r + 1;
		xAdd = -0.9*xSum_vel;
		xVel =  0.7*xVel;
	}
	else if(x + r >= ILI9341_LCD_PIXEL_WIDTH - 1) //right border
	{
		b->x = ILI9341_LCD_PIXEL_WIDTH - r - 1;
		xAdd = -0.9*xSum_vel;
		xVel =  0.7*xVel;
	}

	//y-borders check:
	if( (y - r > 0) && (y + r < ILI9341_LCD_PIXEL_HEIGHT - 1) ) //inside border
		b->y = y;
	else if(y - r <= 0) //upper border
	{
		b->y = r + 1;
		yAdd = -0.9*ySum_vel;
		yVel =  0.7*yVel;
	}
	else if(y + r >= ILI9341_LCD_PIXEL_HEIGHT - 1) //lower border
	{
		b->y = ILI9341_LCD_PIXEL_HEIGHT - r - 1;
		yAdd = -0.9*ySum_vel;
		yVel =  0.7*yVel;
	}
}

void Decrease(int *val)
{
	if(abs(*val) > 1)
		*val /=1.15;
	else
		*val = 0;
}

//used as friction imitation
void Draw_Ball(struct Ball* b)
{
	int x = b->x;
	int y = b->y;
	int r = b->r;
	BSP_LCD_SetTextColor(b->color);
	BSP_LCD_DrawCircle(x,y,r);
}

void Init_Ball(struct Ball* b, int x_, int y_, int color_, struct Ball* eraser_)
{
	b->color  = color_;
	b->r      = RADIUS;
	b->x      = x_;
	b->y      = y_;
	b->eraser = eraser_;
}

void Erase_Ball(struct Ball* b) { Draw_Ball(b->eraser); }

void Update_Ball(struct Ball* b)
{
	if ( (HAL_GetTick() - current_tick) >= 10)
	{
		HAL_TIM_Base_Start_IT(&htim6);
		int x,y,r;
		current_tick = HAL_GetTick();
		BSP_GYRO_GetXYZ(Axes_Data);

		xData_act = Translate(Axes_Data[1] - xError_average);
		yData_act = Translate(Axes_Data[0] - yError_average);
		xDiff += xData_act - xData_prev;
		yDiff += yData_act - yData_prev;

		//condition to avoid noise
		if (abs(xDiff) >=1)
		{
			xVel += Downgrade(xDiff);
		}
		if (abs(yDiff) >= 1)
		{
			yVel += Downgrade(yDiff);
		}

		if ( (HAL_GetTick() - tick_decrease) >= 50)
		{
			Decrease(&xAdd);
			Decrease(&yAdd);
			tick_decrease = HAL_GetTick();
		}

		xSum_vel = xVel + xAdd;
		ySum_vel = yVel + yAdd;

		r = RADIUS;
		x = b->x + xSum_vel;
		y = b->y + ySum_vel;

		b->eraser->x = b->x;
		b->eraser->y = b->y;

		Bodred_Check(x,y,r,b);

		//inter-frame drawing condition
		if( (b->eraser->x != b->x) || (b->eraser->y != b->y) )
			Erase_Ball(b);
		Draw_Ball(b);

		xData_prev = xData_act;
		yData_prev = yData_act;
		HAL_TIM_Base_Stop_IT(&htim6);
	}

}
