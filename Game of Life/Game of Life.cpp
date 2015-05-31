// Game of Life.cpp : main project file.

#include "stdafx.h"
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <math.h>
#include <stdio.h>
#include <vector>
#include <unordered_map>

using std::vector;

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

class cell{
public:
	bool alive = false;
	BYTE neighbours = 0;
};

#define i2d(x,y,w)(x+y*w)
#define d2i(x,y,w)(x+y*w)
//typedef std::pair<int, int> coords;
//
//template <class T>
//inline void hash_combine(std::size_t & seed, const T & v)
//{
//	std::hash<T> hasher;
//	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//}
//namespace std
//{
//	template<typename S, typename T> struct hash<pair<S, T>>
//	{
//		inline size_t operator()(const pair<S, T> & v) const
//		{
//			size_t seed = 0;
//			::hash_combine(seed, v.first);
//			::hash_combine(seed, v.second);
//			return seed;
//		}
//	};
//}

//std::unordered_map<coords, BYTE> cells;
//std::list<coords> alivelist;
//std::list<cell*> dielist;

bool keys[256];
bool mouseDown[3];
bool paused = true;
GLuint texture;

const int width = 1000, height = 1000;
GLubyte img[width*height];
 cell cells[width*height];

 std::list<cell> alivelist;

int cx = width / 2, cy = height / 2;
int leftx = 0, rightx = width, topy = 0, bottemy = height;

int program;
int count = 0;


void setupprogrm();
void checktick();

void HandleKeys(){

}
void drawscene(){
	HandleKeys();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0, 0, -1);
	glPointSize(10);
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_QUADS);		//glColor3f(1, 0, 0);
	glTexCoord2f(1, 0);		glVertex3f(width, 0, 0);
	glTexCoord2f(1, 1);		glVertex3f(width, height, 0);
	glTexCoord2f(0, 1);		glVertex3f(0, height, 0);
	glTexCoord2f(0, 0);		glVertex3f(0, 0, 0);
	glEnd();

	if (!paused){
		checktick();
		count++;
	}
	//Sleep(200);
	
	if (count == 1000){
		glutExit();
	}
	glutSwapBuffers();
}
GLuint LoadTexture(){
	GLuint image;
	glGenTextures(1, &image);
	glBindTexture(GL_TEXTURE_2D, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, img);
	return image;
}
float random2(float max, float min){
	float r = ((float)rand()) / (float)RAND_MAX;
	return r*(max - min) + min;
}
char *textFileRead(char *fn) {
	FILE *fp;
	char *content = NULL;
	int count = 0;
	if (fn != NULL) {
		fp = fopen(fn, "rt");
		if (fp != NULL) {
			fseek(fp, 0, SEEK_END);
			count = ftell(fp);
			rewind(fp);
			if (count > 0) {
				content = (char *)malloc(sizeof(char) * (count + 1));
				count = fread(content, sizeof(char), count, fp);
				content[count] = '\0';
			}
			fclose(fp);
		}
	}
	return content;
}
void checkLinkandValidationErrors(int Program){
	GLint val;
	glGetProgramiv(Program, GL_LINK_STATUS, &val);
	if (val == GL_FALSE) {
		char infolog[1024];
		glGetProgramInfoLog(Program, 1024, NULL, infolog);
		printf(infolog);
	}
	else {
		glValidateProgram(Program);
		glGetProgramiv(Program, GL_VALIDATE_STATUS, &val);
		if (val == GL_FALSE) {
			char infolog[1024];
			glGetProgramInfoLog(Program, 1024, NULL, infolog);
			printf(infolog);
		}
		else {
			printf("Successfully linked program \n");
		}
	}
}
void setupprogrm(){
	glLinkProgram(program);
	checkLinkandValidationErrors(program);
	glUseProgram(program);
}
void checkComplierErrors(int Shader){
	GLint val;
	glGetShaderiv(Shader, GL_COMPILE_STATUS, &val);
	if (val == GL_FALSE){
		char infolog[1024];
		glGetShaderInfoLog(Shader, 1024, NULL, infolog);
		printf(infolog);
	}
	else{
		printf("Shader Compile Sucess \n");
	}
}
void LoadVShader(char *Shader){
	GLuint Vert = glCreateShader(GL_VERTEX_SHADER);
	char *vshader;
	vshader = textFileRead(Shader);
	const char *vs = vshader;

	glShaderSource(Vert, 1, &vs, NULL);
	glCompileShader(Vert);
	checkComplierErrors(Vert);
	glAttachShader(program, Vert);
	free(vshader);
}
void LoadFShader(char *Shader){
	GLuint Frag = glCreateShader(GL_FRAGMENT_SHADER);
	char *fshader;
	fshader = textFileRead(Shader);
	const char *fs = fshader;

	glShaderSource(Frag, 1, &fs, NULL);
	glCompileShader(Frag);
	checkComplierErrors(Frag);
	glAttachShader(program, Frag);
	free(fshader);
}
void specialkeyups(int key, int x, int y){
	keys[key] = false;
}
void specialkeys(int key, int x, int y){
	keys[key] = true;
	if (key == GLUT_KEY_F1){
		paused = !paused;
		//checktick();
	}
}
void keyups(unsigned char key, int x, int y){
	keys[key] = false;
}
void keyevent(unsigned char key, int x, int y){
	keys[key] = true;
	if (key == 27){
		glutExit();
	}
}
void checktick(){
	for (int i = 0; i < height; ++i){
		for (int j = 0; j < width; ++j){
			if (cells[i2d(j, i, width)].alive){
				int lx = j - 1, rx = j + 1,
					uy= i - 1, dy = i + 1;

				if (lx < 0){
					lx = width-1;
				}
				else if (j + 1 >= width){
					rx = 0;
				}
				if (i == 0){
					uy = height-1;
				}
				else if (i + 1 >= height){
					dy = 0;
				}
				cells[i2d(lx, uy, width)].neighbours++;
				cells[i2d(lx, i, width)].neighbours++;
				cells[i2d(lx, dy, width)].neighbours++;

				cells[i2d(j, uy, width)].neighbours++;
				cells[i2d(j, dy, width)].neighbours++;

				cells[i2d(rx, uy, width)].neighbours++;
				cells[i2d(rx, i, width)].neighbours++;
				cells[i2d(rx, dy, width)].neighbours++;
			}
		}
	}
	for (int i = 0; i < width*height; ++i){
		int ii = cells[i].neighbours;
		cells[i].neighbours = 0;
		if (cells[i].alive){
			if (!(ii == 2 || ii == 3)){
				cells[i].alive = false;
				img[i] = 255;
			}
		}
		else{
			if (ii == 3){
				cells[i].alive = true;
				img[i] = 0;
			}
		}
		
	}
	texture = LoadTexture();
}

void init(){
	glClearColor(0, 0, 0, 0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	for (int i = 0; i < width*height; ++i){
		if (random2(100, 0) >= 50){
			img[i] = 255;
			cells[i].alive = false;
		}
		else{
			img[i] = 0;
			cells[i].alive = true;
		}

	}
	texture = LoadTexture();
	//LoadVShader("Vertexshader.vert");
	//LoadFShader("Fragshader.frag");
	//setupprogrm();
//	texture = LoadTexture();
}

void fillmousecell(int x, int y){
	//adjusted for zoom
	int ax, ay;
	double dx = (double)(rightx - leftx) / (double)width;
	double dy = (double)(bottemy - topy) / (double)height;
	ax = (x * dx) + leftx;
	ay = (y * dy) + topy;

	img[i2d(ax, ay, width)] = 0;
	cells[i2d(ax, ay, width)].alive = true;
//	alivelist.push_back(coords(ax, ay));
	texture = LoadTexture();
}
//state 0 down, 1up
void mouseevent(int button, int state,int x, int y){
	if (state == 0){
		fillmousecell(x, y);
	}
	if (state == 1){
		mouseDown[button] = false;
	}
}
void mousemove(int x, int y){
	if (mouseDown){
		fillmousecell(x, y);
	}
}
void reshape(int w, int h){
	//if (h == 0)
	//	h = 1;
	//float ratio = 1.0f* w / h;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	glOrtho(leftx, rightx, bottemy, topy, 0.1, 100);
	glMatrixMode(GL_MODELVIEW);
}
void mwheel(int buton, int dir, int x, int y){
	if (dir > 0){
		int xx = width / x;
		int yy = height / y;
		int ncx = x, ncy = y;
		int zoomn = 50;
		int newx = ncx - cx,
			newy = ncy - cy;
		if (abs(newx) < zoomn){
			leftx += newx;
			rightx += newx;
		}
		else{
			int sgnx = sgn(newx);
			leftx += zoomn * sgnx;
			rightx += zoomn * sgnx;
		}
		if (abs(newy) < zoomn){
			topy += newy;
			bottemy += newy;
		}
		else{
			int sgny = sgn(newy);
			topy += zoomn * sgny;
			bottemy += zoomn * sgny;
		}

		if (leftx + zoomn < rightx - zoomn &&
			topy + zoomn < bottemy - zoomn){

			leftx += zoomn;
			rightx -= zoomn;
			topy += zoomn;
			bottemy -= zoomn;
		}
		reshape(width, height);
	}
	else{

	}
}
int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("OGLTemplate");
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
	}
	fprintf(stderr, "Using GLEW: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stderr, "Vendor:: %s\n", glGetString(GL_VENDOR));
	fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stderr, "Version: %s\n", glGetString(GL_VERSION));
	init();

	glutDisplayFunc(drawscene);
	glutReshapeFunc(reshape);
	glutIdleFunc(drawscene);

	glutKeyboardFunc(keyevent);
	glutKeyboardUpFunc(keyups);
	glutSpecialUpFunc(specialkeyups);
	glutSpecialFunc(specialkeys);
	glutMouseWheelFunc(mwheel);
	glutMouseFunc(mouseevent);
	glutMotionFunc(mousemove);
	
	glutMainLoop();
	return 0;
}
