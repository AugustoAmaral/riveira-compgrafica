#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

GLenum doubleBuffer;

#define VERTICES_DO_POLIGONO 4 // vertices do poligono
#define PI 3.141572			   // Pi (3.14)
#define ZERO 0.00001		   // Zero absoluto
#define LARGURA_TELA_X 800	   // largura da tela
#define ALTURA_TELA_Y 600	   // altura da tela
#define TITULO_TELA "Trabalho Ausberto - Basic Glut"

// Presets do programa
#define DISTANCIA_DO_EIXO 70.0				// distancia do eixo até o ponto
#define LIMITE_DISTANCIA_MOUSE_VERTICE 10.0 // A distancia entre o clique do usuario e a vertice

// constantes de desenho
#define GROSSURA_DA_LINHA_DO_EIXO 1
#define COR_DA_LINHA 1.0, 0.0, 0.0
#define COR_DO_POLIGONO 0.0, 0.0, 0.0
#define COR_DE_FUNDO 1.0, 1.0, 1.0, 0.0
#define COR_DA_VERTICE_SELECIONADA 0.0, 1.0, 0.0
#define TAMANHO_FEEDBACK_VERTICE_SELECIONADA 5

// constantes de manipulacao do mouse
#define ESCALA_CISALHA 0.0001f // Quanto maior, mais facil sera a cisalha

// Cria a estrutura de poligono e cria uma variavel chamado poligono que tem vertices
struct polygon
{
	float v[3]; // Vertice
} poligono[VERTICES_DO_POLIGONO];

// Tamanho da tela (x,y)
int windW, windH;

// Operação selecionada pelo usuário
/*
0 - Nenhuma operação
1 - Mover
2 - Rotacionar
3 - Escalar
4 - Cisalhar
5 - TODO: Espelhar
*/
int operacaoSelecionada = 0;

// Como o openGL vai desenhar (GL_FILL, GL_LINE)
int tipoDesenhoPoligono = GL_FILL; // Define se vai ser linha ou preenchido

// Vertice selecioanda pelo usuário
int verticeSelecionada = -1; // inicializada com -1

float centroDoPoligono[3]; // Global - Centro do poligono ou centro de alguma coisa
float gAng = 0.0f;		   // Angulo global ??

// Calcula a posicao do vertice nos eixo
void CalcularPosicaoVertice(float distancia, float angulo, float ponto[3])
{
	ponto[0] = distancia * cos(angulo);
	ponto[1] = distancia * sin(angulo);
	ponto[2] = 0.0;
}

// Funçao que detecta se o usuario clicou no vertice para executar a transformacao
int SelecionarVertice(int posX, int posY)
{
	int vertice;	 // Vertice a ser checada
	float distancia; // valor da distancia calculada entre o clique e o vertice

	verticeSelecionada = -1; // reseta a seleção de vertices

	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		distancia = sqrt(pow((poligono[vertice].v[0] - posX), 2.0) + pow((poligono[vertice].v[1] - posY), 2.0));
		if (distancia < LIMITE_DISTANCIA_MOUSE_VERTICE)
		{
			verticeSelecionada = vertice;
			break;
		}
	}
	return verticeSelecionada;
}

void InicializarPoligono(void)
{
	int vertice;
	float angulo;

	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		angulo = (float)(vertice) * (2.0 * PI) / ((float)(VERTICES_DO_POLIGONO));
		CalcularPosicaoVertice(DISTANCIA_DO_EIXO, angulo, poligono[vertice].v);
	}

	// calcula o angulo b�sico de rotacao
	// não sei ainda
	gAng = (2.0f * PI) / 180.0f;
}

// Funçao que desenha o poligono ????
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
	glBegin(GL_POLYGON);
	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice += 1)
		glVertex2fv(poligono[vertice].v);
	glEnd();
}

void DesenharPontoNoVertice(void)
{
	glColor3f(COR_DA_VERTICE_SELECIONADA);
	glPointSize(TAMANHO_FEEDBACK_VERTICE_SELECIONADA);
	glBegin(GL_POINTS);
	glVertex2fv(poligono[verticeSelecionada].v);
	glEnd();
}

void DesenharTudo(void)
{
	glClearColor(COR_DE_FUNDO);
	glClear(GL_COLOR_BUFFER_BIT);

	DesenharEixo();
	DesenharPoligono();

	if (verticeSelecionada > -1)
		DesenharPontoNoVertice();

	if (doubleBuffer)
		glutSwapBuffers();
	else
		glFlush();
}

void MudarTipoDesenho(int option)
{
	if (tipoDesenhoPoligono == GL_LINE)
		tipoDesenhoPoligono = GL_FILL;
	else
		tipoDesenhoPoligono = GL_LINE;
	glutPostRedisplay();
}

void SelecionarOperacao(int option)
{
	operacaoSelecionada = option;
	glutPostRedisplay();
}

void CriarMenu()
{
	int idMenu = glutCreateMenu(SelecionarOperacao);
	glutAddMenuEntry("Nenhuma", 0);
	glutAddMenuEntry("Mover", 1);
	glutAddMenuEntry("Rotacionar", 2);
	glutAddMenuEntry("Escalar", 3);
	glutAddMenuEntry("Cisalhar", 4);

	glutCreateMenu(MudarTipoDesenho);
	glutAddMenuEntry("Linha/Solido", 0);
	glutAddSubMenu("Transformar", idMenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void MoverPoligono(float posX, float posY)
{
	int vertice;
	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		poligono[vertice].v[0] += posX;
		poligono[vertice].v[1] += posY;
	}
}

void CalcularCentroPoligono(void)
{
	int vertice;
	centroDoPoligono[0] = centroDoPoligono[1] = 0.0f;
	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		centroDoPoligono[0] += poligono[vertice].v[0];
		centroDoPoligono[1] += poligono[vertice].v[1];
	}
	centroDoPoligono[0] /= VERTICES_DO_POLIGONO;
	centroDoPoligono[1] /= VERTICES_DO_POLIGONO;
}

void MoverPoligonoCentro(int t)
{
	// Move o poligono para o centro e depois move para a posição original
	// mover para o centro quando t => -1
	// move devolta quando t => 1
	int vertice;
	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		poligono[vertice].v[0] += (t * centroDoPoligono[0]);
		poligono[vertice].v[1] += (t * centroDoPoligono[1]);
	}
}

void Rotacionar(float posX, float posY)
{
	int vertice;
	float direcao, angulo, ponto[3];

	CalcularCentroPoligono();
	MoverPoligonoCentro(-1);

	// Dierção calcula para qual lado o poligono está sendo rotacionado
	direcao = poligono[verticeSelecionada].v[1] * posX - poligono[verticeSelecionada].v[0] * posY;

	angulo = gAng;
	if (direcao > 0.0f)
		angulo = -1.0f * gAng;

	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		ponto[0] = poligono[vertice].v[0];
		ponto[1] = poligono[vertice].v[1];
		poligono[vertice].v[0] = ponto[0] * cos(angulo) - ponto[1] * sin(angulo);
		poligono[vertice].v[1] = ponto[0] * sin(angulo) + ponto[1] * cos(angulo);
	}
	MoverPoligonoCentro(1);
}

void Escalar(float posX, float posY)
{
	int vertice;
	float escalarX, escalarY;

	CalcularCentroPoligono();
	MoverPoligonoCentro(-1);

	escalarX = escalarY = 1.0f;

	// Verifica a distancia em X/Y entre o mouse e o ponto selecionado
	if (fabs(poligono[verticeSelecionada].v[0]) > 0.01f)
		escalarX = 1.0f + posX / poligono[verticeSelecionada].v[0];
	if (fabs(poligono[verticeSelecionada].v[1]) > 0.01f)
		escalarY = 1.0f + posY / poligono[verticeSelecionada].v[1];

	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		poligono[vertice].v[0] *= escalarX;
		poligono[vertice].v[1] *= escalarY;
	}

	MoverPoligonoCentro(1);
}

void Cisalhar(float posX, float posY)
{
	int vertice;
	float distorcaoX, distorcaoY, ponto[3];

	distorcaoX = ESCALA_CISALHA * posX;
	distorcaoY = ESCALA_CISALHA * posY;

	if (posX > posY)
	{
		if (fabs(poligono[verticeSelecionada].v[0]) > 0.1f)
			distorcaoX = posX / poligono[verticeSelecionada].v[0];
	}
	else if (fabs(poligono[verticeSelecionada].v[1]) > 0.1f)
		distorcaoY = posY / poligono[verticeSelecionada].v[1];

	CalcularCentroPoligono();
	MoverPoligonoCentro(-1);

	for (vertice = 0; vertice < VERTICES_DO_POLIGONO; vertice++)
	{
		ponto[0] = poligono[vertice].v[0];
		ponto[1] = poligono[vertice].v[1];
		poligono[vertice].v[0] = ponto[0] + ponto[1] * distorcaoX;
		poligono[vertice].v[1] = ponto[0] * distorcaoY + ponto[1];
	}

	MoverPoligonoCentro(1);
}

void MovimentoMouse(int posX, int posY)
{
	float distanciaXEixoVertice, distanciaYEixoVertice;

	if (verticeSelecionada > -1)
	{

		// Move a referencia do mouse pro meio da tela
		posX = posX - windW;
		posY = windH - posY;

		// Calcula a distancia entre o vertice selecionado e o mouse
		distanciaXEixoVertice = posX - poligono[verticeSelecionada].v[0];
		distanciaYEixoVertice = posY - poligono[verticeSelecionada].v[1];

		switch (operacaoSelecionada)
		{
		case 1:
			MoverPoligono(distanciaXEixoVertice, distanciaYEixoVertice);
			break;
		case 2:
			Rotacionar(distanciaXEixoVertice, distanciaYEixoVertice);
			break;
		case 3:
			Escalar(distanciaXEixoVertice, distanciaYEixoVertice);
			break;
		case 4:
			Cisalhar(distanciaXEixoVertice, distanciaYEixoVertice);
			break;
		}
		DesenharTudo();
	}
}

void SelecaoMouse(int botaoSelecionado, int estadoBotao, int posX, int posY)
{
	// Resetar o vertice selecionado
	verticeSelecionada = -1;

	if (botaoSelecionado == GLUT_LEFT && estadoBotao == GLUT_DOWN)
	{
		// Move a referencia do mouse pro meio da tela
		posX = posX - windW;
		posY = windH - posY;

		verticeSelecionada = SelecionarVertice(posX, posY);
	}
}

int main(int argc, char **argv)
{
	GLenum type;

	glutInit(&argc, argv);
	type = GLUT_RGB;
	type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;

	glutInitDisplayMode(type);
	glutInitWindowSize(LARGURA_TELA_X, ALTURA_TELA_Y);
	glutCreateWindow(TITULO_TELA);

	InicializarPoligono();
	glutReshapeFunc(Reshape);
	glutDisplayFunc(DesenharTudo);
	glutMotionFunc(MovimentoMouse);
	glutMouseFunc(SelecaoMouse);
	CriarMenu();
	glutMainLoop();

	return (0);
}
