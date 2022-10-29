/* Vasile George-Cristian
* Gr. 331
* Proiect 1 lab - tema 'Stol de pasari'
*/

#include <windows.h>  // biblioteci care urmeaza sa fie incluse
#include <stdlib.h> // necesare pentru citirea shader-elor
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <GL/glew.h> // glew apare inainte de freeglut
#include <GL/freeglut.h> // nu trebuie uitat freeglut.h
#include "loadShaders.h"
#include "Utils.cpp"

// Din biblioteca glm
#include "glm/glm.hpp"  
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

using namespace std;

GLuint
	VaoId,
	VboId,
	ColorBufferId,
	ProgramId,
	myMatrixLocation,
	matrScaleLocation,
	matrTranslLocation,
	matrRotlLocation,
	codColLocation;

int codCol;
float PI = 3.141592, angle = 0;
float tx = 0; float ty = 0;
float width = 1200, height = 750;
glm::mat4
resizeMatrix, myMatrix;

void SendVariables();
float GetRandFloat(float min, float max);

const int birdNr = 20;
class Bird
{
public:
	/// <summary>
	/// Used in wing animation to switch transitions between up and down
	/// </summary>
	bool wingsGoingDown = true;
	/// <summary>
	/// Current scale of the wings
	/// </summary>
	float wingScale = 1.0;
	/// <summary>
	/// 'Speed' of wing scaling
	/// </summary>
	float wingScaleStep = 0.002;
	/// <summary>
	/// used in scaling a bird's wings
	/// </summary>
	glm::mat4 wingScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0, 1.0, 0.0));
	/// <summary>
	/// Used in moving bird up/down slightly on wing flaps
	/// </summary>
	float verticalOffsetOnWingFlap = 0;

	/// <summary>
	/// Offset at which a bird is drawn, relative to 0.0 + current horizontal movement matrix
	/// </summary>
	glm::mat4 positionOffsetMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));
	float horizontalOffset = 0;
	float verticalOffset = 0;
	// ^ hold horizontal and vertical offset because I don't know how to access them

	/// <summary>
	/// used in moving all the birds from left to right
	/// </summary>
	static glm::mat4 movementMatrix;

	static float horizontalMovementOffset;
	static const float horizontalMovementStep;

	/// <summary>
	/// Used in making birds swap place
	/// </summary>
	float targetHorizontalOffset = 0;
	float targetVerticalOffset = 0;
	float horizontalSwapStep = 0;
	float verticalSwapStep = 0;
	bool goingUp = false;
	bool goingRight = false;

	/// <summary>
	/// Resets birds offset, switch-place vectors, and re-randomizes positions and wings.
	/// </summary>
	static void ResetBirds()
	{
		horizontalMovementOffset = 0;
		RandomizeBirds();
		for (auto& bird : birds) {
			bird.targetHorizontalOffset = 0;
			bird.targetVerticalOffset = 0;
			bird.horizontalSwapStep = 0;
			bird.verticalSwapStep = 0;
			bird.goingUp = false;
			bird.goingRight = false;
		}
		//make birds swap places

		int nrSwaps = rand() % 6;
		for (int i = 0; i < nrSwaps; i++)
			SwapBirdPositions(i * 2, i * 2 + 1);
	}

	static void MoveBirds()
	{
		movementMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(horizontalMovementOffset, 0.0, 0.0)); // controleaza translatia paralel cu Ox
		horizontalMovementOffset = horizontalMovementOffset + horizontalMovementStep;

		if (horizontalMovementOffset > width + maxXOffset + 60)
			ResetBirds();

		glutPostRedisplay();
	}

	static void DrawBirds() 
	{
		for (auto& bird : birds)
			bird.DrawBird();
	}

private:
	static const float maxWingScale;
	static const float minWingScale;
	static const int minXOffset = 80;
	static const int maxXOffset = 800;
	static const int minYOffset = 100;
	static const int maxYOffset = 650;
	static const float verticalMovementStep;

	static void SwapBirdPositions(int index1, int index2) 
	{
		const int nrSteps = 5000;

		//will finish swapping places in nrSteps draw calls
		float verticalStep = abs(birds[index1].verticalOffset - birds[index2].verticalOffset) / nrSteps;
		float horizontalStep = abs(birds[index1].horizontalOffset - birds[index2].horizontalOffset) / nrSteps;

		birds[index1].targetVerticalOffset = birds[index2].verticalOffset;
		birds[index2].targetVerticalOffset = birds[index1].verticalOffset;
		birds[index1].targetHorizontalOffset = birds[index2].horizontalOffset;
		birds[index2].targetHorizontalOffset = birds[index1].horizontalOffset;

		birds[index1].goingUp = (birds[index1].verticalOffset <= birds[index2].verticalOffset);
		birds[index2].goingUp = !birds[index1].goingUp;

		birds[index1].goingRight = (birds[index1].horizontalOffset <= birds[index2].horizontalOffset);
		birds[index2].goingRight = !birds[index1].goingRight;

		if (birds[index1].goingUp)
			birds[index1].verticalSwapStep = verticalStep;
		else
			birds[index1].verticalSwapStep = -verticalStep;

		birds[index2].verticalSwapStep = -birds[index1].verticalSwapStep;

		if (birds[index1].goingRight)
			birds[index1].horizontalSwapStep = horizontalStep;
		else
			birds[index1].horizontalSwapStep = -horizontalStep;

		birds[index2].horizontalSwapStep = -birds[index1].horizontalSwapStep;

		//randomize speeds
		birds[index1].horizontalSwapStep *= GetRandFloat(0.5, 1.5);
		birds[index2].horizontalSwapStep *= GetRandFloat(0.5, 1.5);
		birds[index1].verticalSwapStep *= GetRandFloat(0.5, 1.5);
		birds[index2].verticalSwapStep *= GetRandFloat(0.5, 1.5);
	}

	static void RandomizeBirds()
	{
		srand(time(NULL));
		for (auto& bird : birds)
		{
			bird.GenerateRandomWings();
			bird.GenerateRandomOffset();
		}
	}

	/// <summary>
	/// Generates random wing initial positions so the wings on every bird aren't synced. Also sets the 'step', which will make the birds flap their wings at different intervals.
	/// </summary>
	/// <param name=""></param>
	void GenerateRandomWings()
	{
		//set random initial wing angles and movements
		wingScale = GetRandFloat(minWingScale, maxWingScale);
		wingsGoingDown = (rand() % 2 == 1) ? true : false;
		wingScaleStep = GetRandFloat(0.0015, 0.003);
	}
	/// <summary>
	/// Generates random offsets for bird positions
	/// </summary>
	/// <param name=""></param>
	void GenerateRandomOffset() {
		int randX = rand() % (maxXOffset - minXOffset + 1) + minXOffset;
		int randY = rand() % (maxYOffset - minYOffset + 1) + minYOffset;

		randX = -randX;	//horizontal offset must be negative
		for (auto& bird : birds) {
			if (abs(randX - bird.horizontalOffset) <= 50 && abs(randY - bird.verticalOffset) <= 50)
			{
				GenerateRandomOffset();
				return;
			}
		}

		horizontalOffset = randX;
		verticalOffset = randY;
		positionOffsetMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(horizontalOffset, verticalOffset, 0.0));
	}

	/// <summary>
	/// Moves the wings of a bird, and sets the vertical offset when a bird flaps it's wings
	/// </summary>
	void MoveWings()
	{
		if (wingsGoingDown) {
			wingScale -= wingScaleStep;
			verticalOffsetOnWingFlap += verticalMovementStep;

			if (wingScale <= minWingScale)
				wingsGoingDown = false;
		}
		else {
			wingScale += wingScaleStep;
			verticalOffsetOnWingFlap -= verticalMovementStep;

			if (wingScale >= maxWingScale)
				wingsGoingDown = true;
		}
	}

	/// <summary>
	/// Moves a bird's offset until it reaches the desired position
	/// </summary>
	/// <param name=""></param>
	void ChangeOffsets() {
		if (horizontalSwapStep != 0 || verticalSwapStep != 0) 
		{
			verticalOffset += verticalSwapStep;
			horizontalOffset += horizontalSwapStep;

			if (goingUp) 
			{
				if (verticalOffset >= targetVerticalOffset)
					verticalSwapStep = 0;
			}
			else 
			{
				if (verticalOffset <= targetVerticalOffset)
					verticalSwapStep = 0;
			}

			if (goingRight)
			{
				if (horizontalOffset >= targetHorizontalOffset)
					horizontalSwapStep = 0;
			}
			else 
			{
				if (horizontalOffset <= targetHorizontalOffset)
					horizontalSwapStep = 0;
			}

			positionOffsetMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(horizontalOffset, verticalOffset, 0.0));
		}
	}

	/// <summary>
	/// Draws a bird using the required shapes
	/// </summary>
	void DrawBird()
	{
		ChangeOffsets();
		MoveWings();
		glm::mat4 VerticalOffset = glm::translate(glm::mat4(1.0f), glm::vec3(0, verticalOffsetOnWingFlap, 0.0)); // offset pasare #7

		codCol = 3;
		myMatrix = resizeMatrix * Bird::movementMatrix * positionOffsetMatrix * VerticalOffset;
		SendVariables();
		//draw body
		glDrawArrays(GL_POLYGON, 4, 9);

		codCol = 4;
		SendVariables();
		//draw beak
		glDrawArrays(GL_TRIANGLES, 11, 3);

		codCol = 5;
		SendVariables();
		glPointSize(2);
		//draw eye
		glDrawArrays(GL_POINTS, 14, 1);

		wingScaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0, wingScale, 0.0)); // scales the wings vertically

		codCol = 3;
		myMatrix = resizeMatrix * Bird::movementMatrix * positionOffsetMatrix * VerticalOffset * wingScaleMatrix;
		SendVariables();
		//draw wing
		glDrawArrays(GL_POLYGON, 15, 8);
	}
} birds[birdNr];

float Bird::horizontalMovementOffset = 0.0;
const float Bird::horizontalMovementStep = 0.2;
const float Bird::maxWingScale = 1.0;
const float Bird::minWingScale = 0.05;
const float Bird::verticalMovementStep = 0.01;
glm::mat4 Bird::movementMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

float GetRandFloat(float min, float max) 
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

void displayMatrix()
{
	for (int ii = 0; ii < 4; ii++)
	{
		for (int jj = 0; jj < 4; jj++)
			cout << myMatrix[ii][jj] << "  ";
		cout << endl;
	};
	cout << "\n";
};

void CreateVBO(void)
{
	// varfurile 
	GLfloat Vertices[] = {
		// varfuri poligon fundal
		0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 750.0f, 0.0f, 1.0f,
		1200.0f, 750.0f, 0.0f, 1.0f,
		1200.0f, 0.0f, 0.0f, 1.0f,

		// pasare - poligon convex corp
		45.0f, 6.0f, 0.0f, 1.0f,
		15.0f, 5.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
		15.0f, -5.0f, 0.0f, 1.0f,
		25.0f, -6.0f, 1.0f, 1.0f,
		35.0f, -6.0f, 1.0f, 1.0f,
		45.0f, -5.0f, 0.0f, 1.0f,
		52.5f, -2.5f, 0.0f, 1.0f,
		52.5f, 3.0f, 0.0f, 1.0f,

		// pasare - varf #3 triunghi cioc
		60.0f, 0.0f, 0.0f, 1.0f,

		// pasare - varf ochi
		48.75f, 2.0f, 0.0f, 1.0f,
		
		//pasare - poligon convex aripi
		40.0f, 0.0f, 0.0f, 1.0f,
		40.0f, 6.0f, 0.0f, 1.0f,
		35.0f, 12.0f, 0.0f, 1.0f,
		25.0f, 17.0f, 0.0f, 1.0f,
		10.0f, 25.0f, 0.0f, 1.0f,
		0.0f, 28.0f, 0.0f, 1.0f,
		-10.0f, 30.0f, 0.0f, 1.0f,
		25.0f, 0.0f, 0.0f, 1.0f,
	};

	// culorile varfurilor
	GLfloat Colors[] = {
		// varfuri poligon fundal
		0.529f, 0.808f, 0.922f, 1.0f,
		0.992f, 0.722f, 0.750f, 1.0f,
		0.529f, 0.808f, 0.922f, 1.0f,
		0.529f, 0.808f, 0.922f, 1.0f,

	};

	// se creeaza un buffer nou
	glGenBuffers(1, &VboId);
	// este setat ca buffer curent
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	// punctele sunt "copiate" in bufferul curent
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO
	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);
	// se activeaza lucrul cu atribute; atributul 0 = pozitie
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// un nou buffer, pentru culoare
	glGenBuffers(1, &ColorBufferId);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors), Colors, GL_STATIC_DRAW);
	// atributul 1 =  culoare
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}
void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("proj_1_shader.vert", "proj_1_shader.frag");
	glUseProgram(ProgramId);
}
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f); // culoarea de fond a ecranului
	CreateVBO();
	CreateShaders();
	codColLocation = glGetUniformLocation(ProgramId, "codCuloare");
	myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");
	Bird::ResetBirds();
	glEnable(GL_POINT_SMOOTH);
}

void SendVariables()
{
	glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
	glUniform1i(codColLocation, codCol);
}

//--------------------------------------------------------------------- ONLINE SOURCE: https://www.geeksforgeeks.org/program-to-draw-circles-using-mouse-moves-in-opengl/
void DrawCircle(float cx, float cy, float r)
{
	glColor3f(0.0f, 1.0f, 1.0f);

	// Begin the pointer
	glBegin(GL_POLYGON);

	// Iterate through all the
	// 360 degrees
	for (int i = 0; i < 360; i++) {

		// Find the angle
		float angle_theta = i * PI / 180;
		glVertex2f(cx + r * cos(angle_theta),
			cy + r * sin(angle_theta));
	}

	// Sets vertex
	glEnd();
}
// ----------------------------------------------------------------------

void DrawEllipse(float cx, float cy, float rx, float ry) 
{
	glColor3f(0.0f, 1.0f, 1.0f);
	glBegin(GL_POLYGON);

	// Iterate through all the
	// 360 degrees
	for (int i = 0; i < 360; i++) {

		// Find the angle
		float angle_theta = i * PI / 180;
		glVertex2f(cx + rx * cos(angle_theta),
			cy + ry * sin(angle_theta));
	}

	glEnd();
}

float xCloud1 = 750;
float yCloud1 = 500;
float xCloud2 = 250;
float yCloud2 = 400;

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	// TO DO: schimbati transformarile (de exemplu deplasarea are loc pe axa Oy sau pe o alta dreapta)
	resizeMatrix = glm::ortho(0.0f, width, 0.0f, height); // scalam, "aducem" scena la "patratul standard" [-1,1]x[-1,1]

	// Desenare fundal
	myMatrix = resizeMatrix;
	codCol = 0;
	SendVariables();

	glDrawArrays(GL_POLYGON, 0, 4);

	//Desenare 'soare'
	codCol = 1;
	SendVariables();
	DrawCircle(0, 750, 100);

	//Desenare 'nori'
	codCol = 2;
	SendVariables();

	AnimateClouds(yCloud1, yCloud2);
	//nor #1
	DrawEllipse(xCloud1, yCloud1, 50, 45);
	DrawEllipse(xCloud1 + 35, yCloud1 + 25, 45, 45);
	DrawEllipse(xCloud1 + 75, yCloud1 + 35, 45, 45);
	DrawEllipse(xCloud1 + 125, yCloud1 - 10, 50, 50);
	DrawEllipse(xCloud1 + 75, yCloud1 - 35, 35, 35);
	DrawEllipse(xCloud1 + 40, yCloud1 - 20, 35, 35);

	//nor #2
	DrawEllipse(xCloud2, yCloud2, 55, 55);
	DrawEllipse(xCloud2 + 55, yCloud2 + 25, 35, 35);
	DrawEllipse(xCloud2 + 120, yCloud2 + 30, 55, 55);
	DrawEllipse(xCloud2 + 180, yCloud2 - 10, 35, 35);
	DrawEllipse(xCloud2 + 100, yCloud2 - 20, 80, 40);

	//Desenare pasari
	Bird::MoveBirds();
	Bird::DrawBirds();

	glutSwapBuffers();
	glFlush();
}
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(0, 50);
	glutInitWindowSize(1200, 750);
	glutCreateWindow("Stol de pasari");
	glewInit();
	Initialize();
	glutDisplayFunc(RenderFunction);
	glutCloseFunc(Cleanup);
	glutMainLoop();
}

