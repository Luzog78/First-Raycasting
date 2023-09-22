#include <stdio.h>
#include <SDL2/SDL.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_MAX_INDEX (SCREEN_WIDTH * SCREEN_HEIGHT * 4)

typedef struct
{
	int x;
	int y;
} s_point;

typedef struct
{
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} s_color;

typedef struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	Uint8 screen[SCREEN_WIDTH * SCREEN_HEIGHT * 4];
} s_sdl_master;

void screen_draw_pixel(s_sdl_master *master, s_point *point, s_color *color)
{
	int index = (point->y * SCREEN_WIDTH + point->x) * 4;
	if (index >= 0 && index < SCREEN_MAX_INDEX)
		master->screen[index] = color->r;
	if (index + 1 >= 0 && index + 1 < SCREEN_MAX_INDEX)
		master->screen[index + 1] = color->g;
	if (index + 2 >= 0 && index + 2 < SCREEN_MAX_INDEX)
		master->screen[index + 2] = color->b;
	if (index + 3 >= 0 && index + 3 < SCREEN_MAX_INDEX)
		master->screen[index + 3] = color->a;
}

void screen_draw_line(s_sdl_master *master, s_point *point1, s_point *point2, s_color *color)
{
	int dx = point2->x - point1->x;
	int dy = point2->y - point1->y;
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
	float x_inc = dx / (float) steps;
	float y_inc = dy / (float) steps;
	float x = point1->x;
	float y = point1->y;
	for (int i = 0; i <= steps; i++)
	{
		s_point point = {(int) x, (int) y};
		screen_draw_pixel(master, &point, color);
		x += x_inc;
		y += y_inc;
	}
}

void screen_draw_rect(s_sdl_master *master, s_point *point1, s_point *point2, s_color *color, int fill)
{
	for (int y = point1->y; y <= point2->y; y++)
	{
		for (int x = point1->x; x <= point2->x; x++)
		{
			if (fill == 1 || x == point1->x || x == point2->x || y == point1->y || y == point2->y)
			{
				s_point point = {x, y};
				screen_draw_pixel(master, &point, color);
			}
		}
	}
}

void screen_draw_circle(s_sdl_master *master, s_point *point1, int radius, s_color *color, int fill)
{
	int x = radius;
	int y = 0;
	int radius_error = 1 - x;
	while (x >= y)
	{
		if (fill == 1)
		{
			screen_draw_line(master, &(s_point){point1->x - x, point1->y + y}, &(s_point){point1->x + x, point1->y + y}, color);
			screen_draw_line(master, &(s_point){point1->x - y, point1->y + x}, &(s_point){point1->x + y, point1->y + x}, color);
			screen_draw_line(master, &(s_point){point1->x - x, point1->y - y}, &(s_point){point1->x + x, point1->y - y}, color);
			screen_draw_line(master, &(s_point){point1->x - y, point1->y - x}, &(s_point){point1->x + y, point1->y - x}, color);
		}
		else
		{
			screen_draw_pixel(master, &(s_point){point1->x + x, point1->y + y}, color);
			screen_draw_pixel(master, &(s_point){point1->x + y, point1->y + x}, color);
			screen_draw_pixel(master, &(s_point){point1->x - y, point1->y + x}, color);
			screen_draw_pixel(master, &(s_point){point1->x - x, point1->y + y}, color);
			screen_draw_pixel(master, &(s_point){point1->x - x, point1->y - y}, color);
			screen_draw_pixel(master, &(s_point){point1->x - y, point1->y - x}, color);
			screen_draw_pixel(master, &(s_point){point1->x + y, point1->y - x}, color);
			screen_draw_pixel(master, &(s_point){point1->x + x, point1->y - y}, color);
		}
		y++;
		if (radius_error < 0)
		{
			radius_error += 2 * y + 1;
		}
		else
		{
			x--;
			radius_error += 2 * (y - x + 1);
		}
	}
}

int init(s_sdl_master *master)
{
	master->window = NULL;
	master->renderer = NULL;
	master->texture = NULL;
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT * 4; i++)
	{
		master->screen[i] = (i % 4) == 3 ? 255 : 0;
	}

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	master->window = SDL_CreateWindow("Hello World!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
										SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (master->window == NULL)
	{
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
		return 1;
	}

	master->renderer = SDL_CreateRenderer(master->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (master->renderer == NULL)
	{
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		return 1;
	}

	master->texture = SDL_CreateTexture(master->renderer, SDL_PIXELFORMAT_RGBA32,
										SDL_TEXTUREACCESS_STREAMING, SCREEN_WIDTH, SCREEN_HEIGHT);
	if (master->texture == NULL)
	{
		printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
		return 1;
	}

	return 0;
}

void quit(int exit_code, s_sdl_master *master)
{
	if (master->window != NULL)
	{
		SDL_DestroyWindow(master->window);
		if (master->renderer != NULL)
		{
			SDL_DestroyRenderer(master->renderer);
			if (master->texture != NULL)
			{
				SDL_DestroyTexture(master->texture);
			}
		}
	}
	SDL_Quit();
	printf("Exiting with code %d.\n", exit_code);
	exit(0);
}

void update_screen(s_sdl_master *master)
{
	SDL_UpdateTexture(master->texture, NULL, master->screen, SCREEN_WIDTH * 4 * sizeof(Uint8));
	SDL_RenderClear(master->renderer);
	SDL_RenderCopy(master->renderer, master->texture, NULL, NULL);
	SDL_RenderPresent(master->renderer);
}

int main()
{
	s_sdl_master master;

	if (init(&master) != 0)
	{
		quit(1, &master);
	}

	int i = 0;

	while (1)
	{
		SDL_Event event;
		if (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				break;
			}
		}

		if (i >= 255)
			i = -255;
		else
			i += 8;

		s_color color = {0, i < 0 ? -i - 1 : i, 0, 255};
		screen_draw_rect(&master, &(s_point){0, 0}, &(s_point){SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1}, &color, 0);

		update_screen(&master);
	}

	
	quit(0, &master);
	return 0;
}
