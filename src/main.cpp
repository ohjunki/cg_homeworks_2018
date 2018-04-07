#include "cgmath.h"			// slee's simple math library
#include "cgut.h"			// slee's OpenGL utility

//*******************************************************************
// global constants
static const char*	window_name	= "cgbase - Hello triangle!";
static const char*	vert_shader_path = "../bin/shaders/hello.vert";
static const char*	frag_shader_path = "../bin/shaders/hello.frag";

//*******************************************************************
// window objects
GLFWwindow*	window = nullptr;
ivec2		window_size = ivec2( 1280, 1280 );	// initial window size

//*******************************************************************
// OpenGL objects
GLuint		program			= 0;	// ID holder for GPU program
GLuint		vertex_buffer	= 0;	// ID holder for vertex buffer

//*******************************************************************
// global variables
int		frame = 0;		// index of rendering frames
bool    bUseSolidColor = true;
static const int	TRIANGLE_PER_CIRCLE = 36;
static const int	CIRCLE_COUNT = 400;

//*******************************************************************
static const int		DEFAULT_WINDOW_WIDTH = 1280, DEFAULT_WINDOW_HEIGHT = 720;
static float	window_width, window_height;
static float	expand_max_x = 1.0f;
static float	expand_max_y = 1.0f;
float	prv_time, cur_time;

static float	simul_speed = 1;
struct circleInfo {
	vec3 xyz;
	float scaler;
	vec3 velocity;
	vec4 color;
} CircleInfo[CIRCLE_COUNT];

static const float MAX_CIRCLE_SCALER = 0.4f;
static const float MIN_CIRCLE_SCALER = 0.03f;
static const float MAX_CIRCLE_SPEED;

void sortCircleBySpeed() {
	for (int i = 0; i < CIRCLE_COUNT - 1; i++) {
		for (int j = 0; j < CIRCLE_COUNT - 1 - i; j++) {
			if (length(CircleInfo[j].velocity) < length(CircleInfo[j + 1].velocity) ) {
				struct circleInfo temp = { CircleInfo[j].xyz, CircleInfo[j].scaler, CircleInfo[j].velocity, CircleInfo[j].color };
				CircleInfo[j] = CircleInfo[j + 1];
				CircleInfo[j + 1] = temp;
			}
		}
	}
}
vec3 getPrvPosition(struct circleInfo* c) {
	return c->xyz - c->velocity*(cur_time - prv_time)*simul_speed;
}
void overlayRestore(struct circleInfo * t1, struct circleInfo * t2, float timeVector) {
	for (int i = 0; i < 100; i++) {
		t1->xyz = t1->xyz + t1->velocity* (timeVector / 100.0f) ;
		t2->xyz = t2->xyz + t2->velocity* (timeVector / 100.0f);
		if (length(t1->xyz - t2->xyz) >= t1->scaler + t2->scaler) {
			break;
		}
		if (i == 99)
			printf("will make overlap\n");
	}
}
int overlapCnt = 0;
void manageCircleCollision() {
	for (int i = 0; i < CIRCLE_COUNT; i++) {
		struct circleInfo * target = &CircleInfo[i];
		struct circleInfo temp[CIRCLE_COUNT];
		for (int j = i + 1; j < CIRCLE_COUNT; j++) {
			struct circleInfo * tc = &CircleInfo[j];
			float center_len = length(target->xyz - tc->xyz);
			if (center_len <= target->scaler + tc->scaler) {
				if (length(getPrvPosition(target) - getPrvPosition(tc)) > length(target->xyz - tc->xyz)) {
					//overlayRestore(target, tc, prv_time - cur_time);
					vec3 xv_alia = tc->xyz - target->xyz;
					vec3 yv_alia = { xv_alia.y, xv_alia.x*-1 , 0.0f };
					xv_alia /= length(xv_alia);
					yv_alia /= length(yv_alia);
					vec3 xv1 = xv_alia * dot(xv_alia, target->velocity), yv1 = yv_alia * dot(yv_alia, target->velocity);
					vec3 xv2 = xv_alia * dot(xv_alia, tc->velocity), yv2 = yv_alia * dot(yv_alia, tc->velocity);

					vec3 tmp_xv1 = xv1;
					xv1 = ((target->scaler - tc->scaler) / (target->scaler + tc->scaler)) * xv1 + ((tc->scaler * 2) / (target->scaler + tc->scaler))*xv2;
					xv2 = ((tc->scaler - target->scaler) / (target->scaler + tc->scaler))* xv2 + ((target->scaler * 2) / (target->scaler + tc->scaler))*tmp_xv1;
					//vec3 tmp_yv1 = yv1;
					//yv1 = ((target->scaler - tc->scaler) / (target->scaler + tc->scaler)) * yv1 + ((tc->scaler * 2) / (target->scaler + tc->scaler))*yv2;
					//yv2 = ((tc->scaler - target->scaler) / (target->scaler + tc->scaler))* yv2 + ((target->scaler * 2) / (target->scaler + tc->scaler))*tmp_yv1;
					target->velocity = xv1 + yv1;
					tc->velocity = xv2 + yv2;
					overlayRestore(target, tc, cur_time - prv_time);
				}
			}
		}
	}
}
void manageBoundCollision() {
	for (int i = 0; i < CIRCLE_COUNT; i++) {
		struct circleInfo * c = &CircleInfo[i];
		if (c->xyz.x + c->scaler >= expand_max_x && c->velocity.x > 0 ) {
			//c->xyz.x -= 2 * (c->scaler - (expand_max_x - c->xyz.x));
			c->velocity.x *= -1;
		}
		if (c->xyz.y + c->scaler >= expand_max_y && c->velocity.y > 0) {
			//c->xyz.y -= 2 * (c->scaler - (expand_max_y - c->xyz.y));
			c->velocity.y *= -1;
		}
		if (c->xyz.x - c->scaler <= expand_max_x * -1 && c->velocity.x < 0) {
			//c->xyz.x += 2 * (c->scaler - (expand_max_x + c->xyz.x));
			c->velocity.x *= -1;
		}
		if (c->xyz.y - c->scaler <= expand_max_y * -1 && c->velocity.y < 0) {
			//c->xyz.y += 2 * (c->scaler - (expand_max_y + c->xyz.y));
			c->velocity.y *= -1;
		}
	}
}
void updateCirclesInfo() {
	for (int i = 0; i < CIRCLE_COUNT; i++)
		CircleInfo[i].xyz = CircleInfo[i].xyz + CircleInfo[i].velocity*(cur_time - prv_time)*simul_speed;
	manageCircleCollision();
	manageBoundCollision();
	sortCircleBySpeed();
}
void updateVertex() {
}

mat3 getResolutionConvertMatrix() {
	return mat3{
		1 / expand_max_x,	0,		0,
		0	,	1 / expand_max_y,	0,
		0	,	0,					1
	};
}

float rand0to1() {
	return rand() / (float)RAND_MAX;
}

void renderWithUpdate()
{
	// clear screen (with background color) and clear depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// notify GL that we use our own program
	glUseProgram(program);
	
	// bind vertex attributes to your shader program
	const char*	vertex_attrib[] = { "position", "normal", "texcoord" };
	size_t		attrib_size[] = { sizeof(vertex().pos), sizeof(vertex().norm), sizeof(vertex().tex) };
	for (size_t k = 0, kn = std::extent<decltype(vertex_attrib)>::value, byte_offset = 0; k<kn; k++, byte_offset += attrib_size[k - 1])
	{
		GLuint loc = glGetAttribLocation(program, vertex_attrib[k]); if (loc >= kn) continue;
		glEnableVertexAttribArray(loc);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(loc, attrib_size[k] / sizeof(GLfloat), GL_FLOAT, GL_FALSE, sizeof(vertex), (GLvoid*)byte_offset);
	}

	for (int i = 0; i < CIRCLE_COUNT; i++) {
		GLint uloc;
		struct circleInfo * circle = &CircleInfo[i];
		uloc = glGetUniformLocation(program, "circlePosition");			if (uloc > -1) glUniform3fv(uloc, 1, circle->xyz);
		uloc = glGetUniformLocation(program, "circleScaler");			if (uloc > -1) glUniform1f(uloc, circle->scaler);
		uloc = glGetUniformLocation(program, "resultionConvert");		if (uloc > -1) glUniformMatrix3fv(uloc, 1, GL_FALSE, getResolutionConvertMatrix());

		uloc = glGetUniformLocation(program, "bUseSolidColor");		if (uloc>-1) glUniform1i(uloc, bUseSolidColor);
		uloc = glGetUniformLocation(program, "solid_color");		if (uloc > -1) glUniform4fv(uloc, 1, circle->color );	// pointer version
		
		// render vertices: trigger shader programs to process vertex data
		glDrawArrays( GL_TRIANGLES, 0, TRIANGLE_PER_CIRCLE*3 ); // (topology, start offset, no. vertices)
	}
	// swap front and back buffers, and display to screen
	glfwSwapBuffers(window);
}
void updateCircle() {
}
void initCircle() {
	for (int i = 0; i < CIRCLE_COUNT; i++) {
		bool isFind = false;
		while (!isFind) {
			vec3 p = vec3( (rand0to1()*2 -1) *expand_max_x , (rand0to1()*2-1) * expand_max_y, 0);
			float max_scaler = min(expand_max_x - abs(p.x), expand_max_y - abs(p.y));
			if (max_scaler < MIN_CIRCLE_SCALER) continue;
			for (int j = 0; j < i; j++) {
				float temp_scaler = length(CircleInfo[j].xyz - p) - CircleInfo[j].scaler;
				if (temp_scaler <= 0) {
					max_scaler = 0.0f;
					break;
				}
				else if (max_scaler > temp_scaler)
					max_scaler = temp_scaler;
			}
			if (max_scaler != 0 && max_scaler > MIN_CIRCLE_SCALER ) {
				if (max_scaler > MAX_CIRCLE_SCALER) max_scaler = MAX_CIRCLE_SCALER;
				isFind = true;
				CircleInfo[i].xyz = p;
				CircleInfo[i].scaler = MIN_CIRCLE_SCALER + ( rand0to1() )*(max_scaler - MIN_CIRCLE_SCALER);
				CircleInfo[i].velocity = vec3((rand0to1() * 2 - 1)/(CIRCLE_COUNT/20.0f), (rand0to1() * 2 - 1)/(CIRCLE_COUNT/20.0f), 0.0f); //CircleInfo[i].xyz*CircleInfo[i].scaler*5;
				CircleInfo[i].color = vec4(rand0to1(), rand0to1(), rand0to1(), 1.0f);
			}
		}
	}
	sortCircleBySpeed();
}
void reshape( GLFWwindow* window, int width, int height )
{
	// set current viewport in pixels (win_x, win_y, win_width, win_height)
	// viewport: the window area that are affected by rendering 
	window_size = ivec2(width,height);
	window_width = (float)width;
	window_height = (float)height;
	if (width > height) {
		expand_max_x = window_width / window_height;
		expand_max_y = 1;
	} else {
		expand_max_x = 1;
		expand_max_y = window_height / window_width;
	}
	updateCircle();
	glViewport( 0, 0, width, height );
}

void print_help()
{
	printf( "[help]\n" );
	printf( "- press ESC or 'q' to terminate the program\n" );
	printf( "- press F1 or 'h' to see help\n" );
	printf( "- press 'd' to toggle between solid color and texture coordinates\n" );
	printf( "\n" );
}

void keyboard( GLFWwindow* window, int key, int scancode, int action, int mods )
{
	if(action==GLFW_PRESS)
	{
		if(key==GLFW_KEY_ESCAPE||key==GLFW_KEY_Q)	glfwSetWindowShouldClose( window, GL_TRUE );
		else if(key==GLFW_KEY_H||key==GLFW_KEY_F1)	print_help();
		else if(key==GLFW_KEY_D)
		{
			bUseSolidColor = !bUseSolidColor;
			printf( "> using %s\n", bUseSolidColor ? "solid color" : "texture coordinates as color" );
		}
		else if (key == GLFW_KEY_1) {
			if (simul_speed > 8) {
				simul_speed = 1.0f / simul_speed;
			}else
				simul_speed *= 2.0f;
		}
		else if (key == GLFW_KEY_2) {
			simul_speed *= 0.5f;
		}
	}
}

void mouse( GLFWwindow* window, int button, int action, int mods )
{
	if(button==GLFW_MOUSE_BUTTON_LEFT&&action==GLFW_PRESS )
	{
		dvec2 pos; glfwGetCursorPos(window,&pos.x,&pos.y);
		printf( "> Left mouse button pressed at (%d, %d)\n", int(pos.x), int(pos.y) );
	}
}

void motion( GLFWwindow* window, double x, double y )
{
}

bool user_init()
{
	// log hotkeys
	print_help();

	// init GL states
	glClearColor( 39/255.0f, 40/255.0f, 34/255.0f, 1.0f );	// set clear color
	glEnable( GL_CULL_FACE );								// turn on backface cullings
	glEnable( GL_DEPTH_TEST );								// turn on depth test

	// create a vertex array for triangles in default view volume: [-1~1, -1~1, -1~1]
	vertex vertices[TRIANGLE_PER_CIRCLE * 3];
	float ANGLE_ONE_TRIANGLE = 2.0f * PI / TRIANGLE_PER_CIRCLE;
	for (int i = 0; i < TRIANGLE_PER_CIRCLE; i++) {
		vertices[i * 3] = { vec3(0.0f , 0.0f , 0.0f) , vec3(1,0,0), vec2(0.0f) };
		vertices[i * 3 + 1] = { vec3(cos(i*ANGLE_ONE_TRIANGLE), sin(i*ANGLE_ONE_TRIANGLE) , 0.0f) , vec3(1,0,0), vec2(0.0f) };
		vertices[i * 3 + 2] = { vec3(cos((i+1)*ANGLE_ONE_TRIANGLE), sin((i+1)*ANGLE_ONE_TRIANGLE) , 0.0f) , vec3(1,0,0), vec2(0.0f) };
	}
	initCircle();
	// create and update vertex buffer
	glGenBuffers( 1, &vertex_buffer ); // generate a buffer object
	glBindBuffer( GL_ARRAY_BUFFER, vertex_buffer ); // notify GL to use the buffer
	glBufferData( GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW ); // copy data to GPU

	return true;
}

void user_finalize()
{
}

void main( int argc, char* argv[] )
{
	srand((unsigned int)time(0));
	// initialization
	if(!glfwInit()){ printf( "[error] failed in glfwInit()\n" ); return; }

	// create window and initialize OpenGL extensions
	if(!(window = cg_create_window( window_name, window_size.x, window_size.y ))){ glfwTerminate(); return; }
	if(!cg_init_extensions( window )){ glfwTerminate(); return; }	// init OpenGL extensions

	// initializations and validations of GLSL program
	if(!(program=cg_create_program( vert_shader_path, frag_shader_path ))){ glfwTerminate(); return; }	// create and compile shaders/program
	
	// register event callbacks
	glfwSetWindowSizeCallback( window, reshape );	// callback for window resizing events
    glfwSetKeyCallback( window, keyboard );			// callback for keyboard events
	glfwSetMouseButtonCallback( window, mouse );	// callback for mouse click inputs
	glfwSetCursorPosCallback( window, motion );		// callback for mouse movements

	glfwSetWindowSize(window, DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
	if (!user_init()) { printf("Failed to user_init()\n"); glfwTerminate(); return; }					// user initialization vertex buffer를 완성한다. 원1개를 만든다.
																										
	// enters rendering/event loop
	prv_time = (float)glfwGetTime();
	for( frame=0; !glfwWindowShouldClose(window); frame++ )
	{
		cur_time = (float)glfwGetTime();
		glfwPollEvents();	// polling and processing of events
		updateCirclesInfo();
		renderWithUpdate();			// per-frame render
		prv_time = cur_time;
	}
	
	// normal termination
	user_finalize();
	cg_destroy_window(window);
}
