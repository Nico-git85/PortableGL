#include "rsw_math.h"

#define MANGLE_TYPES
#define PORTABLEGL_IMPLEMENTATION
#include "GLObjects.h"


#include <vector>
#include <iostream>
#include <stdio.h>


#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

//#define WIDTH 1280
//#define HEIGHT 960
#define WIDTH 960
#define HEIGHT 960

using namespace std;

using rsw::vec4;
using rsw::vec3;
using rsw::mat4;

vec4 Red(1.0f, 0.0f, 0.0f, 0.0f);
vec4 Green(0.0f, 1.0f, 0.0f, 0.0f);
vec4 Blue(0.0f, 0.0f, 1.0f, 0.0f);

SDL_Window* window;
SDL_Renderer* ren;
SDL_Texture* tex;

u32* bbufpix;

glContext the_Context;	

typedef struct My_Uniforms
{
	mat4 mvp_mat;
	vec4 v_color;
} My_Uniforms;

void cleanup();
void setup_context();
void setup_gl_data();


void normal_vs(float* vs_output, void* vertex_attribs, Shader_Builtins* builtins, void* uniforms);
void normal_fs(float* fs_input, Shader_Builtins* builtins, void* uniforms);

int main(int argc, char** argv)
{
	setup_context();


	srand(10);

	vector<vec3> lines;

	for (int i=0; i < 1000; ++i) {
		lines.push_back(vec3(rsw::rand_float(-1, 1), rsw::rand_float(-1, 1), 0));
	}

	/*
	lines.push_back(vec3(-1, 0, 0));
	lines.push_back(vec3(1, 0, 0));
	lines.push_back(vec3(0, -1, 0));
	lines.push_back(vec3(0, 1, -0));


	lines.push_back(vec3(1.75, 0.25, 0));
	lines.push_back(vec3(-1.25, -0.25, 0));
	*/

	//lines.push_back(vec3(-0.25, 1.50, 0));
	//lines.push_back(vec3(-1.25, -0.75, 0));
	
	//lines.push_back(vec3(1.5, -1, 2));
	//lines.push_back(vec3(-1.5, 1, -2));

	My_Uniforms the_uniforms;
	mat4 identity;

	Buffer triangle(1);
	triangle.bind(GL_ARRAY_BUFFER);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*lines.size(), &lines[0], GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);


	GLuint myshader = pglCreateProgram(normal_vs, normal_fs, 0, NULL, GL_FALSE);
	glUseProgram(myshader);

	set_uniform(&the_uniforms);

	the_uniforms.v_color = Red;
	the_uniforms.mvp_mat = identity; //only necessary in C of course but that's what I want, to have it work as both

	int line_smooth = 0;
	//glEnable(GL_LINE_SMOOTH);


	SDL_Event e;
	bool quit = false;

	unsigned int old_time = 0, new_time=0, counter = 0;
	unsigned int frame_cap_old = 0, frame_cap_new = 0, last_frame = 0;
	float frame_time = 0;
		
	while (!quit) {
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_QUIT)
				quit = true;
			if (e.type == SDL_KEYDOWN) {
				switch (e.key.keysym.sym) {
					case SDLK_l:
						if (line_smooth)
							glDisable(GL_LINE_SMOOTH);
						else
							glEnable(GL_LINE_SMOOTH);

						line_smooth = !line_smooth;
						break;
					default:
						quit = true;
				}
			}
			if (e.type == SDL_MOUSEBUTTONDOWN)
				quit = true;
		}

		new_time = SDL_GetTicks();
		frame_time = (new_time - last_frame)/1000.0f;
		last_frame = new_time;
		
		counter++;
		if (!(counter % 300)) {
			printf("%d  %f FPS\n", new_time-old_time, 300000/((float)(new_time-old_time)));
			old_time = new_time;
			counter = 0;
		}

		
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays(GL_LINES, 0, lines.size());
		//glDrawArrays(GL_LINES, 1000, 6);

		SDL_UpdateTexture(tex, NULL, bbufpix, WIDTH * sizeof(u32));
		//Render the scene
		SDL_RenderClear(ren);
		SDL_RenderCopy(ren, tex, NULL, NULL);
		SDL_RenderPresent(ren);
	}

	cleanup();	

	return 0;
}


void normal_vs(float* vs_output, void* vertex_attribs, Shader_Builtins* builtins, void* uniforms)
{
	*(vec4*)&builtins->gl_Position = *((mat4*)uniforms) * ((vec4*)vertex_attribs)[0];
}

void normal_fs(float* fs_input, Shader_Builtins* builtins, void* uniforms)
{
	*(vec4*)&builtins->gl_FragColor = ((My_Uniforms*)uniforms)->v_color;
}

void setup_context()
{
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_EVERYTHING)) {
		cout << "SDL_Init error: " << SDL_GetError() << "\n";
		exit(0);
	}

	ren = NULL;
	tex = NULL;
	
	SDL_Window* window = SDL_CreateWindow("swrenderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	if (!window) {
		cerr << "Failed to create window\n";
		SDL_Quit();
		exit(0);
	}

	ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	tex = SDL_CreateTexture(ren, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);

	if (!init_glContext(&the_Context, &bbufpix, WIDTH, HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)) {
		puts("Failed to initialize glContext");
		exit(0);
	}
	set_glContext(&the_Context);
}

void cleanup()
{
	free_glContext(&the_Context);

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);
	SDL_DestroyWindow(window);

	SDL_Quit();
}


