/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

static unsigned int programId;

//modalità programma
#define BASE 0;
#define CATMULL_ROM 1;

#define BASE_RENDER 0;
#define ADAPTIVE_SUB 1;

int mode = BASE;
int renderMode = BASE_RENDER;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

using namespace glm;

#define MaxNumPts 3000
float PointArray[MaxNumPts][2];
float CurveArray[MaxNumPts][2];

int NumPts = 0;
int curvePtsIndex = 0;//contatore dei punti della curva per adaptive render

int ncurvepoints = 100;//base curve points

float tolerance = 0.05f;

int currentPointIndex = -1;//variabile per la selezione del control point da spostare

float tension = 0.0f;
float continuity = 0.0f;
float bias = 0.0f;
float alfa = 0.0f; //serve per curve g1

// Window size in pixels
int		width = 500;
int		height = 500;


/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();


void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 'r':
		mode = CATMULL_ROM;
		glutPostRedisplay();
		break;
	case 'a':
		renderMode = ADAPTIVE_SUB;
		glutPostRedisplay();
		break;
	case 'R':
		mode = BASE;
		glutPostRedisplay();
		break;
	case 'A':
		renderMode = BASE;
		glutPostRedisplay();
		break;
	case '1'://C0
		continuity = 0.5f;
		alfa = 0.0f;
		break;
	case '2'://G1
		continuity = 0.0f;
		alfa = 0.5f;
		break;
	case '3'://C1
		continuity = 0.0f;
		alfa = 0.0f;
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}
void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];
		}
	}
}
void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

int findNearestPoint(glm::vec2 screenPos) {
	int nearestPoint = -1;
	float minDistance = std::numeric_limits<float>::max();//massimo numero float
	float point[2];
	for (int i = 0; i < NumPts; i++) {
		point[0] = PointArray[i][0];
		point[1] = PointArray[i][1];
		// Converte le coordinate OpenGL in coordinate dello schermo
		glm::vec2 screenPoint = glm::vec2(point[0], point[1]);
		// Calcola la distanza tra il punto e le coordinate dello schermo
		float distance = glm::length(screenPoint - screenPos);
		// Verifica se questo punto è più vicino del punto precedente
		if (distance < minDistance) {
			nearestPoint = i;
			minDistance = distance;
		}
	}
	return nearestPoint;
}


void myMouseFunc(int button, int state, int x, int y) {
	// Left button presses place a new control point.
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		//(x,y) viewport(0,width)x(0,height)   -->   (xPos,yPos) window(-1,1)x(-1,1)
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		addNewPoint(xPos, yPos);
		glutPostRedisplay();

	}
	//seleziono punto di controllo
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		currentPointIndex = findNearestPoint(glm::vec2(-1.0f + ((float)x) * 2 / ((float)(width)), -1.0f + ((float)(height - y)) * 2 / ((float)(height))));
	}
	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP) {
		currentPointIndex = -1;
	}
}

void mouseDragged(int x, int y) {
	if (currentPointIndex >= 0) {
		// Aggiorna la posizione del punto selezionato con le nuove coordinate dello schermo
		PointArray[currentPointIndex][0] = -1.0f + ((float)x) * 2 / ((float)(width));
		PointArray[currentPointIndex][1] = -1.0f + ((float)(height - y)) * 2 / ((float)(height));
		glutPostRedisplay();
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPts > 0) {
		NumPts--;
	}
}

void addCatmullPoint(float x, float y) {
	if (NumPts == 0) {
		// se non ci sono punti aggiungo semplicemente
		PointArray[NumPts][0] = x;
		PointArray[NumPts][1] = y;
		NumPts++;
	}
	else if (NumPts == 1) {
		// se aggiungo il secondo punto calcolo i punti intermedi
		// in questo caso la continuita ’ e ’ sempre c1
		vec2 p1 = vec2(PointArray[0][0], PointArray[0][1]);
		vec2 p2 = vec2(x, y);
		vec2 r = (1 - tension) * (1 - continuity) * (1 - bias) * (p2 - p1);
		vec2 l = (1 - tension) * (1 - continuity) * (1 + bias) * (p2 - p1); 
		vec2 p1_p = p1 + (r / 3.0f);
		vec2 p2_m = p2 - (l / 3.0f);
		PointArray[NumPts][0] = p1_p.x;
		PointArray[NumPts][1] = p1_p.y;
		NumPts++;
		PointArray[NumPts][0] = p2_m.x;
		PointArray[NumPts][1] = p2_m.y;
		NumPts++;
		PointArray[NumPts][0] = x;
		PointArray[NumPts][1] = y;
		NumPts++;
	}
	else {
		// dal terzo punto in poi occorre aggiustare anche il penultimo
		// punto di controllo fra quelli gia ’ inseriti in base al tipo
		// di continuita ’ desiderata
		vec2 pn_2 = vec2(PointArray[NumPts - 4][0], PointArray[NumPts - 4][1]);
		vec2 pn_1 = vec2(PointArray[NumPts-1][0], PointArray[NumPts - 1][1]);
		vec2 pn = vec2(x, y);
		//sistemo pn_1
		vec2 l = ((1 - tension + alfa) * (1 - continuity) * (1 + bias) * (pn_1 - pn_2) / 2.0f) + ((1 - tension + alfa) * (1 + continuity) * (1 - bias) * (pn - pn_1) / 2.0f);
		vec2 r = ((1 - tension - alfa) * (1 + continuity) * (1 + bias) * (pn_1 - pn_2) / 2.0f) + ((1 - tension - alfa) * (1 - continuity) * (1 - bias) * (pn - pn_1) / 2.0f);
		vec2 pn_1_m = pn_1 - (l / 3.0f);
		vec2 pn_1_p = pn_1 + (r / 3.0f);
		PointArray[NumPts - 2][0] = pn_1_m.x;
		PointArray[NumPts - 2][1] = pn_1_m.y;
		PointArray[NumPts][0] = pn_1_p.x;
		PointArray[NumPts][1] = pn_1_p.y;
		NumPts++;
		//aggiungo pn_m e pn
		l = (1 - tension) * (1 - continuity) * (1 + bias) * (pn - pn_1);
		vec2 pn_m = pn - (l / 3.0f);
		PointArray[NumPts][0] = pn_m.x;
		PointArray[NumPts][1] = pn_m.y;
		NumPts++;
		PointArray[NumPts][0] = x;
		PointArray[NumPts][1] = y;
		NumPts++;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}

	if (mode == 1) {//catmull rom
		addCatmullPoint(x,y);
	}
	else {
		PointArray[NumPts][0] = x;
		PointArray[NumPts][1] = y;
		NumPts++;
	}
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}

void deCasteljau(float t, float* res) {

	float TempArray[MaxNumPts][2];
	int dim = NumPts;
	for (int i = 0; i < dim; i++) {
		TempArray[i][0] = PointArray[i][0];
		TempArray[i][1] = PointArray[i][1];
	}

	//interpolazione lineare
	for (int i = 1; i < dim; i++) {
		for (int j = 0; j < dim - 1; j++) {
			TempArray[j][0] = (1 - t) * TempArray[j][0] + t * TempArray[j+1][0];
			TempArray[j][1] = (1 - t) * TempArray[j][1] + t * TempArray[j+1][1];
		}
	}
	
	res[0] = TempArray[0][0];
	res[1] = TempArray[0][1];
}

int deCasteljauCubic() {
	int ncur = 0;
	if (NumPts == 4 || (NumPts - 1) % 3 == 0) {
		float reducedCurve[4][2];
		int pointsPerCurve = 20;
		for (int num = 0; num+1 < NumPts; num += 3) {
			reducedCurve[0][0] = PointArray[num][0];
			reducedCurve[0][1] = PointArray[num][1];
			reducedCurve[1][0] = PointArray[num + 1][0];
			reducedCurve[1][1] = PointArray[num + 1][1];
			reducedCurve[2][0] = PointArray[num + 2][0];
			reducedCurve[2][1] = PointArray[num + 2][1];
			reducedCurve[3][0] = PointArray[num + 3][0];
			reducedCurve[3][1] = PointArray[num + 3][1];
			int init = num == 0 ? 0 : 1; //non considero 2 volte i punti d'interpolazione
			for (int acc = init; acc <= pointsPerCurve; acc++) {
				float t = (GLfloat)acc / pointsPerCurve;
				for (int i = 1; i < 4; i++) {
					for (int j = 0; j < 3; j++) {
						reducedCurve[j][0] = (1 - t) * reducedCurve[j][0] + t * reducedCurve[j + 1][0];
						reducedCurve[j][1] = (1 - t) * reducedCurve[j][1] + t * reducedCurve[j + 1][1];
					}
				}
				CurveArray[ncur][0] = reducedCurve[0][0];
				CurveArray[ncur][1] = reducedCurve[0][1];
				ncur++;
			}
		}
	}
	return ncur;
}

void suddivisioneAdattiva(float tempArray[MaxNumPts][2], int numPts, float tolerance) {
	vec2 p1 = vec2(tempArray[0][0], tempArray[0][1]);
	vec2 p2 = vec2(tempArray[numPts - 1][0], tempArray[numPts - 1][1]);
	// Calcola la lunghezza del segmento di retta tra p1 e p2
	float lineLength = glm::length(p1 - p2);

	bool test_plan = true;
	for (int i = 1; i < numPts - 1; i++) {
		vec2 p3 = vec2(tempArray[i][0], tempArray[i][1]);
		// Calcola l'area del triangolo formato dai tre punti
		float area = abs((p2[0] - p1[0]) * (p3[1] - p1[1]) - (p3[0] - p1[0]) * (p2[1] - p1[1]));

		// Calcola la distanza tra il punto p3 e la retta passante per p1 e p2
		float distance = area / lineLength;
		if (distance > tolerance) {
			test_plan = false;
		}
	}
	if (test_plan) {
		CurveArray[curvePtsIndex][0] = p1[0];
		CurveArray[curvePtsIndex][1] = p1[1];
		curvePtsIndex++;
		CurveArray[curvePtsIndex][0] = p2[0];
		CurveArray[curvePtsIndex][1] = p2[1];
		curvePtsIndex++;

		return;
	}
	else {
		float subd_1[MaxNumPts][2];
		float subd_2[MaxNumPts][2];
		float t = 0.5;

		subd_1[0][0] = p1[0];
		subd_1[0][1] = p1[1];
		subd_2[numPts - 1][0] = p2[0];
		subd_2[numPts - 1][1] = p2[1];

		vector<vec2> TempArray;
		for (int i = 0; i < numPts; i++) {
			vec2 p = vec2(tempArray[i][0], tempArray[i][1]);
			TempArray.push_back(p);
		}

		

		for (int i = 1; i < numPts; i++) {
			for (int j = 0; j < numPts - 1; j++) {
				TempArray[j][0] = (1 - t) * TempArray[j][0] + t * TempArray[j + 1][0];
				TempArray[j][1] = (1 - t) * TempArray[j][1] + t * TempArray[j + 1][1];
			}

			subd_1[i][0] = TempArray[0][0];
			subd_1[i][1] = TempArray[0][1];

			subd_2[numPts - 1 - i][0] = TempArray[numPts - 1 - i][0];
			subd_2[numPts - 1 - i][1] = TempArray[numPts - 1 - i][1];
		}

		suddivisioneAdattiva(subd_1, numPts,tolerance);
		curvePtsIndex--;
		suddivisioneAdattiva(subd_2, numPts,tolerance);
		return;
	}

}

void suddivisioneAdattivaCubic(float tempArray[MaxNumPts][2], float tolerance) {
	if (NumPts == 4 || (NumPts - 1) % 3 == 0) {
		float reducedCurve[4][2];
		for (int num = 0; num + 1 < NumPts; num += 3) {
			reducedCurve[0][0] = PointArray[num][0];
			reducedCurve[0][1] = PointArray[num][1];
			reducedCurve[1][0] = PointArray[num + 1][0];
			reducedCurve[1][1] = PointArray[num + 1][1];
			reducedCurve[2][0] = PointArray[num + 2][0];
			reducedCurve[2][1] = PointArray[num + 2][1];
			reducedCurve[3][0] = PointArray[num + 3][0];
			reducedCurve[3][1] = PointArray[num + 3][1];

			suddivisioneAdattiva(reducedCurve, 4, tolerance);
		}
	}
}

void mouse_scroll_callback(int button, int dir, int x, int y)
{
	if (renderMode == 1) {//adaptive
		if (dir == 1) // Scorrimento verso l'alto
		{
			tolerance += 0.01f;
			glutPostRedisplay();
		}
		else if (dir == -1 && tolerance>0.01f) // Scorrimento verso il basso
		{
			tolerance -= 0.01f;
			glutPostRedisplay();
		}
	}
}


void init(void)
{
	// VAO for control polygon
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// VAO for curve
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 1) {//draw curve
		if (mode == 0 && renderMode == 0) {//BASE
			float result[2];
			for (int i = 0; i <= ncurvepoints; i++) {
				deCasteljau((GLfloat)i / ncurvepoints, result);
				CurveArray[i][0] = result[0];
				CurveArray[i][1] = result[1];
			}
			glBindVertexArray(VAO_2);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, ncurvepoints + 1);
			glBindVertexArray(0);
		}
		else if (mode == 1 && NumPts > 3 && renderMode == 0) {//CATMULL_ROM BASE
			float result[3];
			int curvePoints=deCasteljauCubic();

			glBindVertexArray(VAO_2);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, curvePoints);
			glBindVertexArray(0);
		}
		else if (mode == 0 && renderMode == 1 && NumPts > 2) {//ADAPTIVE_SUBD
			curvePtsIndex = 0;
			suddivisioneAdattiva(PointArray,NumPts, tolerance);
			glBindVertexArray(VAO_2);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, curvePtsIndex);
			glBindVertexArray(0);
		}
		else if (mode == 1 && NumPts > 3 && renderMode == 1) {//CATMULL_ROM ADAPTIVE
			curvePtsIndex = 0;
			suddivisioneAdattivaCubic(PointArray, tolerance);

			glBindVertexArray(VAO_2);
			glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
			glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
			glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
			glEnableVertexAttribArray(0);
			glLineWidth(0.5);
			glDrawArrays(GL_LINE_STRIP, 0, curvePtsIndex);
			glBindVertexArray(0);
		}
		
	}
	// Draw control polygon
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Draw the control points CP
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	// Draw the line segments between CP
	glLineWidth(2.0);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);
	glBindVertexArray(0);
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(mouseDragged);
	glutMouseWheelFunc(mouse_scroll_callback);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}
