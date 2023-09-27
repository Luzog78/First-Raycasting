#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define PI 3.14159265358979323846

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_MAX_INDEX (SCREEN_WIDTH * SCREEN_HEIGHT * 4)
#define RAYS_AMOUNT 512
#define RAYS_DISPLAY 10
#define RAYS_FOV (PI / 3)
#define RAYS_MAX_DISTANCE 100

typedef struct
{
	float x;
	float y;
} t_vec2;

typedef struct
{
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} t_color;

typedef struct
{
	int width;
	int height;
	int scale;
	Uint8 *array;
} t_sdl_canvas;

typedef struct
{
	int width;
	int height;
	int *array;
} t_level;

typedef struct
{
	t_vec2 position;
	float angle;
	float distance;
	int side;
} t_ray;

typedef struct
{
	t_vec2 position;
	float direction;
	float speed;
	t_ray rays[RAYS_AMOUNT];
} t_player;

typedef struct
{
	SDL_Window *window;
	t_sdl_canvas screen;
	t_sdl_canvas minimap;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	t_level level;
	t_player player;
	double clock;
} t_sdl_master;

void screen_draw_pixel(t_sdl_canvas *canvas, t_vec2 *point, t_color *color)
{
	int size = canvas->width * canvas->height * 4;
	int index = (point->y * canvas->width + point->x) * 4;
	if (index >= 0 && index + 3 < size)
	{
		t_color pixel = {canvas->array[index], canvas->array[index + 1], canvas->array[index + 2], canvas->array[index + 3]};
		canvas->array[index] = (pixel.r * (255 - color->a) + color->r * color->a) / 255;
		canvas->array[index + 1] = (pixel.g * (255 - color->a) + color->g * color->a) / 255;
		canvas->array[index + 2] = (pixel.b * (255 - color->a) + color->b * color->a) / 255;
		canvas->array[index + 3] = 255 - ((255 - pixel.a) * (255 - color->a) / 255);
	}
}

void screen_draw_line(t_sdl_canvas *canvas, t_vec2 *point1, t_vec2 *point2, t_color *color)
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
		t_vec2 point = {(int) x, (int) y};
		screen_draw_pixel(canvas, &point, color);
		x += x_inc;
		y += y_inc;
	}
}

void screen_draw_rect(t_sdl_canvas *canvas, t_vec2 *point1, t_vec2 *point2, t_color *color, int fill)
{
	for (int y = point1->y; y <= point2->y; y++)
	{
		for (int x = point1->x; x <= point2->x; x++)
		{
			if (fill == 1 || x == point1->x || x == point2->x || y == point1->y || y == point2->y)
			{
				t_vec2 point = {x, y};
				screen_draw_pixel(canvas, &point, color);
			}
		}
	}
}

void screen_draw_circle(t_sdl_canvas *canvas, t_vec2 *point1, int radius, t_color *color, int fill)
{
	int x = radius;
	int y = 0;
	int radius_error = 1 - x;
	while (x >= y)
	{
		if (fill == 1)
		{
			screen_draw_line(canvas, &(t_vec2){point1->x - x, point1->y + y}, &(t_vec2){point1->x + x, point1->y + y}, color);
			screen_draw_line(canvas, &(t_vec2){point1->x - y, point1->y + x}, &(t_vec2){point1->x + y, point1->y + x}, color);
			screen_draw_line(canvas, &(t_vec2){point1->x - x, point1->y - y}, &(t_vec2){point1->x + x, point1->y - y}, color);
			screen_draw_line(canvas, &(t_vec2){point1->x - y, point1->y - x}, &(t_vec2){point1->x + y, point1->y - x}, color);
		}
		else
		{
			screen_draw_pixel(canvas, &(t_vec2){point1->x + x, point1->y + y}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x + y, point1->y + x}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x - y, point1->y + x}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x - x, point1->y + y}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x - x, point1->y - y}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x - y, point1->y - x}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x + y, point1->y - x}, color);
			screen_draw_pixel(canvas, &(t_vec2){point1->x + x, point1->y - y}, color);
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

int init(t_sdl_master *master)
{
	master->window = NULL;
	master->renderer = NULL;
	master->texture = NULL;
	master->screen.width = SCREEN_WIDTH;
	master->screen.height = SCREEN_HEIGHT;
	master->screen.scale = 1;
	master->screen.array = malloc(master->screen.width * master->screen.height * 4 * sizeof(Uint8));
	master->level.width = 8;
	master->level.height = 8;
	master->level.array = malloc(master->level.width * master->level.height * sizeof(int));
	master->minimap.width = master->level.width * 24;
	master->minimap.height = master->level.height * 24;
	master->minimap.scale = 1;
	master->minimap.array = malloc(master->minimap.width * master->minimap.height * 4 * sizeof(Uint8));
	master->player.position = (t_vec2){3.5, 5.5};
	master->player.direction = 0;
	master->player.speed = 0.1;
	master->clock = 0;

	if (master->screen.array == NULL || master->minimap.array == NULL || master->level.array == NULL)
	{
		printf("malloc Error.\n");
		return 1;
	}

	for (int i = 0; i < master->screen.width * master->screen.height * 4; i++)
		master->screen.array[i] = (i % 4) == 3 ? 255 : 0;
	for (int i = 0; i < master->minimap.width * master->minimap.height * 4; i++)
		master->minimap.array[i] = (i % 4) == 3 ? 255 : 0;
	for (int i = 0; i < master->level.width * master->level.height; i++)
		master->level.array[i] = (int[]){
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 1, 0, 0, 0, 0, 1,
			1, 0, 1, 0, 0, 0, 0, 1,
			1, 0, 1, 0, 1, 1, 0, 1,
			1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 0, 0, 1,
			1, 1, 1, 1, 1, 1, 1, 1
		}[i];

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

	master->texture = SDL_CreateTexture(master->renderer,
								SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
								SCREEN_WIDTH, SCREEN_HEIGHT);
	if (master->texture == NULL)
	{
		printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
		return 1;
	}

	return 0;
}

void quit(int exit_code, t_sdl_master *master)
{
	if (master->screen.array != NULL)
	{
		free(master->screen.array);
	}
	if (master->minimap.array != NULL)
	{
		free(master->minimap.array);
	}
	if (master->level.array != NULL)
	{
		free(master->level.array);
	}
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

void update_canvas(int size, int width, Uint8 **pixels, t_sdl_canvas *canvas)
{
	for (int y = 0; y < canvas->height * canvas->scale; y++)
	{
		for (int x = 0; x < canvas->width * canvas->scale; x++)
		{
			int index = (((int) (y / canvas->scale)) * canvas->width + ((int) (x / canvas->scale))) * 4 ;
			int window_index = (y * width + x) * 4;
			if (window_index >= 0 && window_index + 3 < size)
			{
				(*pixels)[window_index] = ((*pixels)[window_index] * (255 - canvas->array[index + 3]) + canvas->array[index] * canvas->array[index + 3]) / 255;
				(*pixels)[window_index + 1] = ((*pixels)[window_index + 1] * (255 - canvas->array[index + 3]) + canvas->array[index + 1] * canvas->array[index + 3]) / 255;
				(*pixels)[window_index + 2] = ((*pixels)[window_index + 2] * (255 - canvas->array[index + 3]) + canvas->array[index + 2] * canvas->array[index + 3]) / 255;
				(*pixels)[window_index + 3] = 255 - ((255 - (*pixels)[window_index + 3]) * (255 - canvas->array[index + 3]) / 255);
			}
		}
	}
}

void update_window(t_sdl_master *master)
{
	int size = SCREEN_WIDTH * SCREEN_HEIGHT * 4;
	Uint8 *pixels = malloc(size * sizeof(Uint8));
	if (pixels == NULL)
	{
		printf("malloc Error.\n");
		quit(1, NULL);
	}
	for (int i = 0; i < size; i++)
	{
		pixels[i] = 0;
	}
	update_canvas(size, SCREEN_WIDTH, &pixels, &master->screen);
	update_canvas(size, SCREEN_WIDTH, &pixels, &master->minimap);
	SDL_UpdateTexture(master->texture, NULL, pixels, SCREEN_WIDTH * 4 * sizeof(Uint8));
	SDL_RenderClear(master->renderer);
	SDL_RenderCopy(master->renderer, master->texture, NULL, NULL);
	SDL_RenderPresent(master->renderer);
	free(pixels);
}

void update_minimap(t_sdl_master *master)
{
	for (int i = 0; i < master->minimap.width * master->minimap.height * 4; i++)
	{
		master->minimap.array[i] = (i % 4) == 3 ? 255 : 0;
	}

	for (int y = 0; y < master->level.height; y++)
	{
		for (int x = 0; x < master->level.width; x++)
		{
			int index = y * master->level.width + x;
			if (master->level.array[index] == 1)
			{
				screen_draw_rect(&master->minimap,
					&(t_vec2){x * 24, y * 24}, &(t_vec2){x * 24 + 24, y * 24 + 24}, &(t_color){255, 0, 0, 196}, 1);
			}
		}
	}

	for (int i = 0; i < RAYS_DISPLAY; i++)
	{
		t_ray ray = master->player.rays[i * RAYS_AMOUNT / RAYS_DISPLAY];
		screen_draw_line(&master->minimap,
			&(t_vec2){master->player.position.x * 24, master->player.position.y * 24},
			&(t_vec2){ray.position.x * 24, ray.position.y * 24},
			&(t_color){255, 255, 0, 255});
	}

	screen_draw_line(&master->minimap,
		&(t_vec2){master->player.position.x * 24, master->player.position.y * 24},
		&(t_vec2){master->player.position.x * 24 + 24 * cos(master->player.direction), master->player.position.y * 24 + 24 * sin(master->player.direction)},
		&(t_color){255, 255, 255, 255});
	screen_draw_circle(&master->minimap,
		&(t_vec2){master->player.position.x * 24, master->player.position.y * 24},
		6, &(t_color){0, 255, 255, 255}, 1);
}

void update_screen(t_sdl_master *master)
{
	for (int i = 0; i < master->screen.width * master->screen.height * 4; i++)
	{
		master->screen.array[i] = (i % 4) == 3 ? 255 : 0;
	}

	screen_draw_rect(&master->screen,
		&(t_vec2){0, 0}, &(t_vec2){master->screen.width, master->screen.height / 2},
		&(t_color){0, 128, 255, 255}, 1);
	screen_draw_rect(&master->screen,
		&(t_vec2){0, master->screen.height / 2}, &(t_vec2){master->screen.width, master->screen.height},
		&(t_color){170, 85, 0, 255}, 1);

	for (int i = 0; i < RAYS_AMOUNT; i++)
	{
		t_ray ray = master->player.rays[i];
		float distance = ray.distance * cos(ray.angle - master->player.direction);
		float height = master->screen.height / distance / 1.5;
		float position = (master->screen.height - height) / 2.0;
		float tiling = master->screen.width / (float) RAYS_AMOUNT;
		screen_draw_rect(&master->screen,
			&(t_vec2){i * tiling, position},
			&(t_vec2){i * tiling + tiling, position + height},
			ray.side == 1 ? &(t_color){255, 255, 255, 255} : &(t_color){225, 225, 225, 255}, 1);
	}
}

int handle_collisions(t_sdl_master *master, int depth, t_vec2 position, t_vec2 back, t_vec2 direction)
{
	for (float off_y = -0.2; off_y <= 0.2; off_y += 0.1)
	{
		for (float off_x = -0.2; off_x <= 0.2; off_x += 0.1)
		{
			t_vec2 pos = {position.x + off_x, position.y + off_y};
			if (master->level.array[(int) pos.y * master->level.width + (int) pos.x] == 1)
			{
				if (depth != 0)
				{
					return 1;
				}

				position.x = back.x + direction.x;
				position.y = back.y;

				if (handle_collisions(master, depth + 1, position, back, direction) == 0)
				{
					master->player.position.x = position.x;
					master->player.position.y = position.y;
					return 0;
				}

				position.x = back.x;
				position.y = back.y + direction.y;

				if (handle_collisions(master, depth + 1, position, back, direction) == 0)
				{
					master->player.position.x = position.x;
					master->player.position.y = position.y;
					return 0;
				}

				return 1;
			}
		}
	}
	return 0;
}

void move_player(t_sdl_master *master, float x, float y)
{
	t_vec2 back = {master->player.position.x, master->player.position.y};
	master->player.position.x += x;
	master->player.position.y += y;


	if (handle_collisions(master, 0, master->player.position, back, (t_vec2){x, y}))
	{
		master->player.position.x = back.x;
		master->player.position.y = back.y;
	}
}

int main()
{
	t_sdl_master master;

	if (init(&master) != 0)
	{
		quit(1, &master);
	}

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

		SDL_PumpEvents();
		const Uint8 *state = SDL_GetKeyboardState(NULL);
		if (state[SDL_SCANCODE_ESCAPE])
		{
			break;
		}
		if (state[SDL_SCANCODE_UP])
		{
			move_player(&master,
				master.player.speed * cos(master.player.direction),
				master.player.speed * sin(master.player.direction));
		}
		if (state[SDL_SCANCODE_DOWN])
		{
			move_player(&master,
				-master.player.speed * cos(master.player.direction),
				-master.player.speed * sin(master.player.direction));
		}
		if (state[SDL_SCANCODE_LEFT])
		{
			master.player.direction -= 0.1;
		}
		if (state[SDL_SCANCODE_RIGHT])
		{
			master.player.direction += 0.1;
		}

		for (int i = 0; i < RAYS_AMOUNT; i++)
		{
			float angle = master.player.direction - RAYS_FOV / 2 + i * RAYS_FOV / RAYS_AMOUNT;
			while (angle < 0)
				angle += 2 * PI;
			while (angle >= 2 * PI)
				angle -= 2 * PI;

			float dx, dy;

			t_vec2 vertical_position;
			float atan = -1 / tan(angle);
			vertical_position.y = (int) master.player.position.y + (angle >= PI ? 0 : 1);
			vertical_position.x = master.player.position.x + (master.player.position.y - vertical_position.y) * atan;
			dy = (angle >= PI ? -1 : 1);
			dx = -dy * atan;
			while (vertical_position.y >= 0 && vertical_position.y < master.level.height)
			{
				int index = ((int) (vertical_position.y + (angle >= PI ? -1 : 0))) * master.level.width + (int) (vertical_position.x);
				if (index < 0 || index >= master.level.width * master.level.height)
					break;
				if (master.level.array[index] == 1)
				{
					break;
				}
				vertical_position.y += dy;
				vertical_position.x += dx;
			}
			float vertical_distance = sqrt(pow(vertical_position.x - master.player.position.x, 2) + pow(vertical_position.y - master.player.position.y, 2));

			t_vec2 horizontal_position;
			float ntan = -tan(angle);
			horizontal_position.x = (int) master.player.position.x + (angle >= PI / 2 && angle < 3 * PI / 2 ? 0 : 1);
			horizontal_position.y = master.player.position.y + (master.player.position.x - horizontal_position.x) * ntan;
			dx = (angle >= PI / 2 && angle < 3 * PI / 2 ? -1 : 1);
			dy = -dx * ntan;
			while (horizontal_position.x >= 0 && horizontal_position.x < master.level.width)
			{
				int index = ((int) (horizontal_position.y)) * master.level.width + (int) (horizontal_position.x + (angle >= PI / 2 && angle < 3 * PI / 2 ? -1 : 0));
				if (index < 0 || index >= master.level.width * master.level.height)
					break;
				if (master.level.array[index] == 1)
				{
					break;
				}
				horizontal_position.x += dx;
				horizontal_position.y += dy;
			}
			float horizontal_distance = sqrt(pow(horizontal_position.x - master.player.position.x, 2) + pow(horizontal_position.y - master.player.position.y, 2));

			master.player.rays[i].position = vertical_distance < horizontal_distance ? vertical_position : horizontal_position;
			master.player.rays[i].angle = angle;
			master.player.rays[i].distance = vertical_distance < horizontal_distance ? vertical_distance : horizontal_distance;
			master.player.rays[i].side = vertical_distance < horizontal_distance ? 0 : 1;
		}

		update_minimap(&master);
		update_screen(&master);

		double t = SDL_GetPerformanceCounter() / 1000000.0;
		double fps = 1000 / (t - master.clock);
		if (1)
		{
			printf("FPS: %f\n", fps);
		}

		master.clock = t;
		SDL_Delay(1000 / 120);
		update_window(&master);
	}
	
	quit(0, &master);
	return 0;
}
