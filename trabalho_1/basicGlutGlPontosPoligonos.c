#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

GLenum doubleBuffer;

#define MAX_VERTICES_DO_POLIGONO 4 // vertices do poligono
#define LARGURA_TELA_X 800		   // largura da tela
#define ALTURA_TELA_Y 600		   // altura da tela
#define TITULO_TELA "Trabalho Ausberto - Basic Glut"

// Presets do programa
#define LIMITE_DISTANCIA_MOUSE_VERTICE 10.0 // A distancia entre o clique do usuario e a vertice

// constantes de desenho
#define GROSSURA_DA_LINHA_DO_EIXO 1
#define COR_DA_LINHA 1.0, 0.0, 0.0
#define COR_DO_POLIGONO 0.0, 0.0, 0.0
#define COR_DE_FUNDO 1.0, 1.0, 1.0, 0.0
#define COR_DA_VERTICE_SELECIONADA 0.0, 1.0, 0.0
#define TAMANHO_FEEDBACK_VERTICE_SELECIONADA 5

// Cria a estrutura de poligono e cria uma variavel chamado poligono que tem vertices
struct polygon
{
	float v[3]; // Vertice
} poligono[MAX_VERTICES_DO_POLIGONO];

// Tamanho da tela (x,y)
int windW, windH;

// Como o openGL vai desenhar (GL_POINTS, GL_LINE_LOOP)
int tipoDesenhoPoligono = GL_POINTS; // Define se vai ser o ponto ou linha

// Numero de pontos que o usuário fez
int pontosCriados = 0;

// O poligono foi desenhado?
int poligonoJaDesenhado = 0;

void InicializarPoligono(void)
{
	int i;

	poligonoJaDesenhado = 0;
	pontosCriados = 0;
	tipoDesenhoPoligono = GL_POINTS;

	for (i = 0; i < MAX_VERTICES_DO_POLIGONO; i++)
	{
		poligono[i].v[0] = 0.0f;
		poligono[i].v[1] = 0.0f;
		poligono[i].v[2] = 0.0f;
	}
}

// Funçao que redesenha o polígono dentro da tela
void Reshape(int width, int height)
{
	windW = width / 2;
	windH = height / 2;

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-windW, windW, -windH, windH);

	glMatrixMode(GL_MODELVIEW);
}

void DesenharEixo(void)
{
	// Grossura da linha do eixo
	glLineWidth(GROSSURA_DA_LINHA_DO_EIXO);
	// Cor da linha do eixo
	glColor3f(COR_DA_LINHA);

	// Desenha o eixo X
	glBegin(GL_LINE_STRIP);
	glVertex2f(-windW, 0);
	glVertex2f(windW, 0);
	glEnd();

	// Desenha o eixo Y
	glBegin(GL_LINE_STRIP);
	glVertex2f(0, -windH);
	glVertex2f(0, windH);
	glEnd();
}

void DesenharPoligono(void)
{
	int vertice;

	glColor3f(COR_DO_POLIGONO);
	glPolygonMode(GL_FRONT_AND_BACK, tipoDesenhoPoligono);

	glBegin(tipoDesenhoPoligono);
	for (vertice = 0; vertice < pontosCriados; vertice++)
	{
		glVertex2fv(poligono[vertice].v);
	}
	glEnd();
}

static void DesenharTudo(void)
{
	glClearColor(COR_DE_FUNDO);
	glClear(GL_COLOR_BUFFER_BIT);

	DesenharEixo();
	DesenharPoligono();

	if (doubleBuffer)
		glutSwapBuffers();
	else
		glFlush();
}

void processMenuEvents(int option)
{
	switch (option)
	{
	case 2:
		InicializarPoligono();
		break;
	}
	glutPostRedisplay();
}

void processSubMenuEvents(int option)
{
	if (option == 0)
		tipoDesenhoPoligono = GL_POINTS;
	else if (option == 1)
	{
		tipoDesenhoPoligono = GL_LINE_LOOP;
		poligonoJaDesenhado = 1;
	}

	glutPostRedisplay();
}

static void Args(int argc, char **argv)
{
	GLint i;

	doubleBuffer = GL_FALSE;

	for (i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-sb") == 0)
		{
			doubleBuffer = GL_FALSE;
		}
		else if (strcmp(argv[i], "-db") == 0)
		{
			doubleBuffer = GL_TRUE;
		}
	}
}

void createGLUTMenus()
{
	int menu, submenu;

	submenu = glutCreateMenu(processSubMenuEvents);
	glutAddMenuEntry("Pontos", 0);
	glutAddMenuEntry("Poligono", 1);

	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Limpar", 2);
	glutAddSubMenu("Tipo Objeto", submenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void motion(int x, int y)
{
}

void mouse(int button, int state, int x, int y)
{

	if (state == GLUT_UP)
	{
		printf("\n poligonoJaDesenhado %d ", poligonoJaDesenhado);
		if (button == GLUT_LEFT_BUTTON)
		{
			if (poligonoJaDesenhado == 0)
			{
				x = x - windW;
				y = windH - y;

				//				glColor3f(0.0, 1.0, 0.0);
				glPointSize(3);
				//				glBegin(GL_POINTS);
				//				glVertex2i(x, y);
				//				glEnd();

				poligono[pontosCriados].v[0] = (float)x;
				poligono[pontosCriados].v[1] = (float)y;
				pontosCriados++;
			}
			else
			{
			}
		}
		else if (button == GLUT_RIGHT_BUTTON)
		{
			if (pontosCriados > 0)
			{
				poligonoJaDesenhado = 1;
				tipoDesenhoPoligono = GL_LINE;
			}
		}
	}
	glutPostRedisplay();
}

int main(int argc, char **argv)
{
	GLenum type;

	glutInit(&argc, argv);
	Args(argc, argv);

	type = GLUT_RGB;
	type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;

	glutInitDisplayMode(type);
	glutInitWindowSize(600, 500);
	glutCreateWindow("Basic Program Using Glut and Gl");

	InicializarPoligono();

	glutReshapeFunc(Reshape);
	glutDisplayFunc(DesenharTudo);

	glutMotionFunc(motion);
	glutMouseFunc(mouse);
	//	glutIdleFunc(idle);

	createGLUTMenus();

	glutMainLoop();

	return (0);
}