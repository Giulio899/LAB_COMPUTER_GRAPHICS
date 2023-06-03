// 2D_JUMPING_BALL.cpp : 
//
#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include "HUD_Logger.h"

static unsigned int programId;
#define PI 3.14159265358979323846

unsigned int VAO_CAMPO, VAO_SOLE, VAO_CIELO, VAO_PALLA, VAO_PARETE, VAO_CANESTRO, VAO_NET, VAO_PARTICLE;
unsigned int VBO_CAMPO, VBO_PARETE, VBO_S, VBO_C, VBO_Pa, VBO_CANESTRO, VBO_NET, VBO_PARTICLE, MatProj, MatModel;

// Include GLM; libreria matematica per le opengl
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
using namespace glm;

mat4 Projection;  //Matrice di proiezione
mat4 Model; //Matrice per il cambiamento da Sistema di riferimento dell'oggetto OCS a sistema di riferimento nel Mondo WCS

// viewport size
int width = 1200;
int height = 720;
typedef struct { float x, y, r, g, b, a; } Pointxy;
int Numero_di_pezzi = 128;
int vertices_Campo = 6;
Pointxy* Campo = new Pointxy[vertices_Campo];
int vertices_Cielo = 6;
Pointxy* Cielo = new Pointxy[vertices_Cielo];
int vertices_Parete = 6;
Pointxy* Parete = new Pointxy[vertices_Parete];
int nTriangles_sole = 30;
int vertices_sole = 3 * 2 * nTriangles_sole;
Pointxy* Sole = new Pointxy[vertices_sole];
int nTriangles_palla = 30;
int vertices_palla = 3 * 2 * nTriangles_palla;
Pointxy* Palla = new Pointxy[vertices_palla];
int vertices_Canestro = 18;
Pointxy* Canestro = new Pointxy[vertices_Canestro];
int vertices_Net = 42;
Pointxy* Net = new Pointxy[vertices_Net];

// parametri della palla
int		distacco_da_terra = 0;			// distacco da terra 
float	delta = 15;
double	VelocitaOrizzontale = 0; //velocita orizzontale (pixel per frame)

float	posx = float(width) / 5; //coordinate sul piano della posizione iniziale della palla
float	posy = float(height) * 0.2;

bool pressing_left = false;
bool pressing_right = false;

int g_scale_factor =50;

float g = 9.81*g_scale_factor;
vec2 ballVelocity = vec2(-1.0, -1.0);
float ballVelocityY0 = 0.0f;
vec2 startingPoint = vec2(-1.0, -1.0);
bool moving = false;
float ballTime = 0.0f;

vec4 col_rosso = { 1.0,0.0,0.0,1.0 };
vec4 col_nero = { 0.0,0.0,0.0,1.0 };
vec4 col_bianco = { 1.0,1.0,1.0,1.0 };

int score = 0;
int attempt = 0;
bool update_score = false;

void init();
/// ///////////////////////////////////////////////////////////////////////////////////
///									Gestione eventi
///////////////////////////////////////////////////////////////////////////////////////
void keyboardPressedEvent(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	default:
		break;
	}
}


void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	float	posx = float(width) / 5; //coordinate sul piano della posizione iniziale della palla
	float	posy = float(height) * 0.2;
	init();//per aggiustare gli oggetti in scena
}

void update(int a)
{
	
	if (moving) {

		ballTime += 0.01f;
		ballVelocity.y = ballVelocityY0 - g * ballTime;
		posx = startingPoint.x + ballVelocity.x * ballTime;
		posy = startingPoint.y + ballVelocityY0 * ballTime - 0.5 * g * ballTime * ballTime;

		if ((posx>= width * 0.76 and posx <= width * 0.85) and
			(posy>=height * 0.6  and posy <= height * 0.61) and 
			(ballVelocity.y < 0)) {//ho fatto canestro
			if (update_score) {
				score++;
				update_score = false;
			}
			ballTime = 0.0f;
			ballVelocity.x = 0.0f;
			ballVelocityY0 = ballVelocity.y;
			startingPoint.y = posy;
			startingPoint.x = posx;

			glutPostRedisplay();
			glutTimerFunc(1, update, 0);
		}
		
		else {
			

			//rimbalzo terra
			if (posy <= height * 0.1) {
				ballTime = 0.0f;
				ballVelocity.y = -0.5 * ballVelocity.y;
				ballVelocityY0 = ballVelocity.y;
				startingPoint.y = height * 0.1 + 0.01f;
				startingPoint.x = posx;
			}
			//rimbalzo estremi schermo
			if (posx < 0.0) {
				ballTime = 0.0f;
				startingPoint.x = 0.0;
				startingPoint.y = posy;
				ballVelocity.x = -0.9 * ballVelocity.x;
				ballVelocity.y = 0.5 * ballVelocity.y;
				ballVelocityY0 = ballVelocity.y;
			}
			if (posx > width) {
				ballTime = 0.0f;
				startingPoint.x = (float)width;
				startingPoint.y = posy;
				ballVelocity.x = -0.9 * ballVelocity.x;
				ballVelocity.y = 0.5 * ballVelocity.y;
				ballVelocityY0 = ballVelocity.y;
			}

			//rimbalzo asta
			if ((posx > width * 0.84 and posx < width * 0.86) and
				(posy > height * 0.1 and posy < height * 0.76)) {
				if (ballVelocity.x > 0) {
					ballTime = 0.0f;
					startingPoint.x = (float)width * 0.84;
					startingPoint.y = posy;
					ballVelocity.x = -0.8 * ballVelocity.x;
					ballVelocityY0 = ballVelocity.y;
				}
				else {
					ballTime = 0.0f;
					startingPoint.x = (float)width * 0.86;
					startingPoint.y = posy;
					ballVelocity.x = -0.8 * ballVelocity.x;
					ballVelocityY0 = ballVelocity.y;
				}
			}

			glutPostRedisplay();
			glutTimerFunc(1, update, 0);
		}
		
	}
	else {
		glutTimerFunc(1, update, 0);
	}
}

void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		ballTime = 0.0f;
		moving = false;
		posx = x;
		posy = height-y;
		startingPoint.x = x;
		startingPoint.y = height - y;
		glutPostRedisplay();
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		float factor = 2;
		ballVelocity = vec2((startingPoint.x - x) * factor, (startingPoint.y - (height - y))*factor);
		ballVelocityY0 = (startingPoint.y - (height - y)) * factor;
		moving = true;
		attempt++;
		update_score = true;
	}
	else if (button == GLUT_RIGHT_BUTTON) {
		ballTime = 0.0f;
		moving = false;
		posx = float(width) / 5;
		posy = float(height) * 0.2;
		startingPoint.x = posx;
		startingPoint.y = posy;
		glutPostRedisplay();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////// 





void disegna_pianoxy(float x, float y, float width, float height, vec4 color_top, vec4 color_bot, Pointxy* piano)
{
	piano[0].x = x;	piano[0].y = y;
	piano[0].r = color_bot.r; piano[0].g = color_bot.g; piano[0].b = color_bot.b; piano[0].a = color_bot.a;
	piano[1].x = x + width;	piano[1].y = y;
	piano[1].r = color_top.r; piano[1].g = color_top.g; piano[1].b = color_top.b; piano[1].a = color_top.a;
	piano[2].x = x + width;	piano[2].y = y + height;
	piano[2].r = color_bot.r; piano[2].g = color_bot.g; piano[2].b = color_bot.b; piano[2].a = color_bot.a;

	piano[3].x = x + width;	piano[3].y = y + height;
	piano[3].r = color_bot.r; piano[3].g = color_bot.g; piano[3].b = color_bot.b; piano[3].a = color_bot.a;
	piano[4].x = x;	piano[4].y = y + height;
	piano[4].r = color_top.r; piano[4].g = color_top.g; piano[4].b = color_top.b; piano[4].a = color_top.a;
	piano[5].x = x;	piano[5].y = y;
	piano[5].r = color_bot.r; piano[5].g = color_bot.g; piano[5].b = color_bot.b; piano[5].a = color_bot.a;
}

void disegna_cerchio(int nTriangles, int step, vec4 color_top, vec4 color_bot, Pointxy* Cerchio) {
	int i;
	float stepA = (2 * PI) / nTriangles;

	int comp = 0;
	// step = 1 -> triangoli adiacenti, step = n -> triangoli distanti step l'uno dall'altro
	for (i = 0; i < nTriangles; i += step)
	{
		Cerchio[comp].x = cos((double)i * stepA);
		Cerchio[comp].y = sin((double)i * stepA);
		Cerchio[comp].r = color_top.r; Cerchio[comp].g = color_top.g; Cerchio[comp].b = color_top.b; Cerchio[comp].a = color_top.a;

		Cerchio[comp + 1].x = cos((double)(i + 1) * stepA);
		Cerchio[comp + 1].y = sin((double)(i + 1) * stepA);
		Cerchio[comp + 1].r = color_top.r; Cerchio[comp + 1].g = color_top.g; Cerchio[comp + 1].b = color_top.b; Cerchio[comp + 1].a = color_top.a;

		Cerchio[comp + 2].x = 0.0;
		Cerchio[comp + 2].y = 0.0;
		Cerchio[comp + 2].r = color_bot.r; Cerchio[comp + 2].g = color_bot.g; Cerchio[comp + 2].b = color_bot.b; Cerchio[comp + 2].a = color_bot.a;
		comp += 3;
	}
}

void disegnaCanestro(Pointxy* Canestro) {
	Pointxy* Sostegno;
	int vertici = 6;
	Sostegno = new Pointxy[vertici];
	//base
	disegna_pianoxy(width*0.8, height*0.1, width * 0.1, height * 0.02, col_nero, col_nero, Sostegno);
	int cont = 0;
	for (int i = 0; i < 6; i++)
	{
		Canestro[cont + i].x = Sostegno[i].x;
		Canestro[cont + i].y = Sostegno[i].y;
		Canestro[cont + i].r = Sostegno[i].r;	Canestro[cont + i].g = Sostegno[i].g;	Canestro[cont + i].b = Sostegno[i].b; Canestro[cont + i].a = Sostegno[i].a;
	}
	//ferro
	disegna_pianoxy(width * 0.76, height * 0.6, width * 0.09, height * 0.01, col_rosso, col_rosso, Sostegno);
	cont = 6;
	for (int i = 0; i < 6; i++)
	{
		Canestro[cont + i].x = Sostegno[i].x;
		Canestro[cont + i].y = Sostegno[i].y;
		Canestro[cont + i].r = Sostegno[i].r;	Canestro[cont + i].g = Sostegno[i].g;	Canestro[cont + i].b = Sostegno[i].b; Canestro[cont + i].a = Sostegno[i].a;
	}
	//asta
	disegna_pianoxy(width * 0.84, height * 0.1, width * 0.02, height * 0.65, col_nero, col_nero, Sostegno);
	cont = 12;
	for (int i = 0; i < 6; i++)
	{
		Canestro[cont + i].x = Sostegno[i].x;
		Canestro[cont + i].y = Sostegno[i].y;
		Canestro[cont + i].r = Sostegno[i].r;	Canestro[cont + i].g = Sostegno[i].g;	Canestro[cont + i].b = Sostegno[i].b; Canestro[cont + i].a = Sostegno[i].a;
	}
}

void disegnaNet(Pointxy* Net) {
	Pointxy* Sostegno;
	int vertici = 6;
	Sostegno = new Pointxy[vertici];
	//4 tratti verticali
	disegna_pianoxy(width * 0.76, height * 0.5, width * 0.005, height * 0.1, col_bianco, col_bianco, Sostegno);
	int cont = 0;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}
	disegna_pianoxy(width * 0.78, height * 0.5, width * 0.005, height * 0.1, col_bianco, col_bianco, Sostegno);
	cont = 6;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}
	disegna_pianoxy(width * 0.805, height * 0.5, width * 0.005, height * 0.1, col_bianco, col_bianco, Sostegno);
	cont = 12;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}
	disegna_pianoxy(width * 0.825, height * 0.5, width * 0.005, height * 0.1, col_bianco, col_bianco, Sostegno);
	cont = 18;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}


	//3 tratti orizzontali
	disegna_pianoxy(width * 0.76, height * 0.525, width * 0.07, height * 0.01, col_bianco, col_bianco, Sostegno);
	cont = 24;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}

	disegna_pianoxy(width * 0.76, height * 0.55, width * 0.07, height * 0.01, col_bianco, col_bianco, Sostegno);
	cont = 30;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}
	disegna_pianoxy(width * 0.76, height * 0.575, width * 0.07, height * 0.01, col_bianco, col_bianco, Sostegno);
	cont = 36;
	for (int i = 0; i < 6; i++)
	{
		Net[cont + i].x = Sostegno[i].x;
		Net[cont + i].y = Sostegno[i].y;
		Net[cont + i].r = Sostegno[i].r;	Net[cont + i].g = Sostegno[i].g;	Net[cont + i].b = Sostegno[i].b; Net[cont + i].a = Sostegno[i].a;
	}
}

double lerp(double a, double b, double amount) {
	//Interpolazione lineare tra a e b secondo amount
	return (1 - amount) * a + amount * b;
}

void disegna_palla(int nTriangles, Pointxy* Palla) {
	//Costruisco la geometria della palla ed i suoi colori
	vec4 col_bottom = { 1.0, 0.8, 0.0, 1.0 };
	disegna_cerchio(nTriangles, 1, col_rosso, col_bottom, Palla);
}

void disegna_sole(int nTriangles, Pointxy* Sole) {
	int i, cont;
	Pointxy* OutSide;
	int vertici = 3 * nTriangles;
	OutSide = new Pointxy[vertici];

	vec4 col_top_sole = { 1.0, 1.0, 1.0, 1.0 };
	vec4 col_bottom_sole = { 1.0, 0.8627, 0.0, 1.0 };
	disegna_cerchio(nTriangles, 1, col_top_sole, col_bottom_sole, Sole);

	col_top_sole = { 1.0, 1.0, 1.0, 0.0 };
	col_bottom_sole = { 1.0, 0.8627, 0.0, 1.0 };
	disegna_cerchio(nTriangles, 1, col_top_sole, col_bottom_sole, OutSide);

	cont = 3 * nTriangles;
	for (i = 0; i < 3 * nTriangles; i++)
	{
		Sole[cont + i].x = OutSide[i].x;
		Sole[cont + i].y = OutSide[i].y;
		Sole[cont + i].r = OutSide[i].r; Sole[cont + i].g = OutSide[i].g; Sole[cont + i].b = OutSide[i].b; Sole[cont + i].a = OutSide[i].a;
	}
}

void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader_C_M.glsl";
	char* fragmentShader = (char*)"fragmentShader_C_M.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}
void init(void)
{
	Projection = ortho(0.0f, float(width), 0.0f, float(height));
	MatProj = glGetUniformLocation(programId, "Projection");
	MatModel = glGetUniformLocation(programId, "Model");

	//Init the logger
	HUD_Logger::get()->set_text_box_height(130);
	HUD_Logger::get()->set_text_color(col_nero.r, col_nero.g, col_nero.b, col_nero.a);

	//Costruzione geometria e colori del CIELO
	vec4 col_top = { 0.3,0.6,1.0,1.0 };
	vec4 col_bottom = { 0.3,0.6,1.0,1.0 };
	disegna_pianoxy(0, height * 0.8, width, height * 0.2 , col_bottom, col_top, Cielo);
	//Generazione del VAO del Cielo
	glGenVertexArrays(1, &VAO_CIELO);
	glBindVertexArray(VAO_CIELO);
	glGenBuffers(1, &VBO_C);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
	glBufferData(GL_ARRAY_BUFFER, vertices_Cielo * sizeof(Pointxy), &Cielo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori della PARETE
	col_top = { 1.0, 0.647, 0.0, 1.0 };
	col_bottom = { 1.0, 0.647, 0.0, 1.0 };
	disegna_pianoxy(0, height * 0.2, width, height * 0.6, col_bottom, col_top, Parete);
	//Generazione del VAO della Parete
	glGenVertexArrays(1, &VAO_PARETE);
	glBindVertexArray(VAO_PARETE);
	glGenBuffers(1, &VBO_PARETE);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_PARETE);
	glBufferData(GL_ARRAY_BUFFER, vertices_Parete * sizeof(Pointxy), &Parete[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori del CAMPO
	col_top = { 0.5, 0.5, 0.5, 1.0 };
	col_bottom = { 0.5, 0.5, 0.5, 1.0 };
	disegna_pianoxy(0, 0, width, height * 0.2, col_bottom, col_top, Campo);
	glGenVertexArrays(1, &VAO_CAMPO);
	glBindVertexArray(VAO_CAMPO);
	glGenBuffers(1, &VBO_CAMPO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_CAMPO);
	glBufferData(GL_ARRAY_BUFFER, vertices_Campo * sizeof(Pointxy), &Campo[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori del SOLE
	//Genero il VAO del SOLE
	disegna_sole(nTriangles_sole, Sole);
	glGenVertexArrays(1, &VAO_SOLE);
	glBindVertexArray(VAO_SOLE);
	glGenBuffers(1, &VBO_S);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
	glBufferData(GL_ARRAY_BUFFER, vertices_sole * sizeof(Pointxy), &Sole[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori della PALLA
	disegna_palla(nTriangles_palla, Palla);
	glGenVertexArrays(1, &VAO_PALLA);
	glBindVertexArray(VAO_PALLA);
	glGenBuffers(1, &VBO_Pa);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_Pa);
	glBufferData(GL_ARRAY_BUFFER, vertices_palla * sizeof(Pointxy), &Palla[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori del CANESTRO
	disegnaCanestro(Canestro);
	glGenVertexArrays(1, &VAO_CANESTRO);
	glBindVertexArray(VAO_CANESTRO);
	glGenBuffers(1, &VBO_CANESTRO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_CANESTRO);
	glBufferData(GL_ARRAY_BUFFER, vertices_Canestro * sizeof(Pointxy), &Canestro[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Costruzione geometria e colori della RETE
	disegnaNet(Net);
	glGenVertexArrays(1, &VAO_NET);
	glBindVertexArray(VAO_NET);
	glGenBuffers(1, &VBO_NET);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_NET);
	glBufferData(GL_ARRAY_BUFFER, vertices_Net * sizeof(Pointxy), &Net[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	//Definisco il colore assegnato allo schermo
	glClearColor(0.0, 0.0, 0.0, 1.0);

	

	glutSwapBuffers();
}

void drawScene(void)
{
	
	glUniformMatrix4fv(MatProj, 1, GL_FALSE, value_ptr(Projection));
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(programId);

	//Disegna cielo
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CIELO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Cielo);
	glBindVertexArray(0);

	// Disegna sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
	Model = scale(Model, vec3(30.0, 30.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_SOLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_sole / 2);
	//Disegna Alone del sole
	Model = mat4(1.0);
	Model = translate(Model, vec3(float(width) * 0.5, float(height) * 0.8, 0.0));
	Model = scale(Model, vec3(80.0, 80.0, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glDrawArrays(GL_TRIANGLES, vertices_sole / 2, vertices_sole / 2);
	glBindVertexArray(0);

	//Disegna parete
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_PARETE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Parete);
	glBindVertexArray(0);

	//Disegna campo
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CAMPO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Campo);
	glBindVertexArray(0);

	// Disegna palla (palla)
	// larghezza effettiva in pixel della palla
	double bwidth = 60 *((double)width/1200);
	// altezza effettiva in pixel della palla
	double bheight = 60 * ((double)height / 720);
	glBindVertexArray(VAO_PALLA);
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); //wireframe
	//Matrice per il cambiamento del sistema di riferimento per la  PALLA
	Model = mat4(1.0);
	Model = translate(Model, vec3(posx, posy, 0.0f));
	Model = scale(Model, vec3(float(bwidth) / 2, float(bheight) / 2, 1.0));
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_palla / 2);
	glBindVertexArray(0);

	//Disegna rete
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_NET);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Net);
	glBindVertexArray(0);

	//Disegna canestro
	Model = mat4(1.0);
	glUniformMatrix4fv(MatModel, 1, GL_FALSE, value_ptr(Model));
	glBindVertexArray(VAO_CANESTRO);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, vertices_Canestro);
	glBindVertexArray(0);

	vector<string> score_vec{};
	score_vec.push_back(" ");
	score_vec.push_back("Punteggio: " + std::to_string(score));
	score_vec.push_back("Tentativi: " + std::to_string(attempt));
	HUD_Logger::get()->printInfo(score_vec, 20, height);

	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("2D Animation");
	glutDisplayFunc(drawScene);
	//Evento tastiera tasto premuto
	glutKeyboardFunc(keyboardPressedEvent);
	glutMouseFunc(myMouseFunc);
	glutReshapeFunc(resizeWindow);

	//gestione animazione
	glutTimerFunc(10, update, 0);
	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glEnable(GL_BLEND);
	glEnable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glutMainLoop();
}