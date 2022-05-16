#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

#define MAXIMO_DE_VERTICES 30 // numero maximo de v�rtices do poligono (espa�o disponivel)
#define PI 3.141572

// Distancia maxima entre o clique do mouse do usuário e o vértice do poligono
#define DISTANCIA_MAXIMA_SELECIONAR_VERTICE 10.0

GLenum doubleBuffer;

struct poligono
{
    float v[3];
} pontos[MAXIMO_DE_VERTICES];

// pontos poligonos
int numeroDeVertices = 0;
int poligonoJaDesenhado = 0;
//-----------------

int tamanhoDaJanelaX, tamanhoDaJanelaY; // comprimento da janela: Width  e Height

int operacaoSelecionada = 0;         // tipo de operacao: O: gera poligono; 1: transalacao; 2:...
int tipoDoPoligono;                  // poligo por vertices ou por arestas
int indiceDoVerticeSelecionado = -1; // numero de v�rtices do poligono definido
int numeroDePontosCriados = 0;
float anguloDeRotacao = 0.0f; // angulo para rotacao
float matrizDeBase[3][3];

int SelecionarVertice(int x, int y)
{
    int contador;
    float distancia;
    indiceDoVerticeSelecionado = -1;
    for (contador = 0; contador < numeroDePontosCriados; contador++)
    {
        distancia = sqrt(pow((pontos[contador].v[0] - x), 2.0) + pow((pontos[contador].v[1] - y), 2.0));
        if (distancia < DISTANCIA_MAXIMA_SELECIONAR_VERTICE)
        {
            indiceDoVerticeSelecionado = contador;
            break;
        }
    }
    return indiceDoVerticeSelecionado;
}

void init(void)
{
    int i;
    tipoDoPoligono = GL_POINTS;
    poligonoJaDesenhado = 0;
    numeroDeVertices = 0;
    numeroDePontosCriados = 0;

    for (i = 0; i < MAXIMO_DE_VERTICES; i++)
    {
        pontos[i].v[0] = 0.0f;
        pontos[i].v[1] = 0.0f;
        pontos[i].v[2] = 1.0f;
    }

    anguloDeRotacao = (2.0f * PI) / 180.0f;
}

static void Reshape(int cumprimento, int altura)
{
    tamanhoDaJanelaX = cumprimento / 2;
    tamanhoDaJanelaY = altura / 2;

    glViewport(0, 0, cumprimento, altura);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-tamanhoDaJanelaX, tamanhoDaJanelaX, -tamanhoDaJanelaY, tamanhoDaJanelaY);

    glMatrixMode(GL_MODELVIEW);
}

void DesenharEixos(void)
{
    glLineWidth(1);

    // vertical line
    glBegin(GL_LINE_STRIP);
    glColor3f(1.0, 0.0, 0.0);
    glVertex2f(-tamanhoDaJanelaX, 0);
    glVertex2f(tamanhoDaJanelaX, 0);
    glEnd();

    // horizontal line

    glBegin(GL_LINE_STRIP);
    glColor3f(0.0, 0.0, 1.0);
    glVertex2f(0, -tamanhoDaJanelaY);
    glVertex2f(0, tamanhoDaJanelaY);
    glEnd();
}

void DesenharPoligono(void)
{
    int contador;

    glColor3f(0.0, 0.0, 0.0);
    glPolygonMode(GL_FRONT_AND_BACK, tipoDoPoligono);

    glBegin(tipoDoPoligono);
    for (contador = 0; contador < numeroDePontosCriados; contador++)
        glVertex2fv(pontos[contador].v);

    glEnd();
}

void DesenharTela(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);

    DesenharEixos();
    DesenharPoligono();

    if (indiceDoVerticeSelecionado > -1)
    {
        glColor3f(1.0, 0.0, 0.0);
        glPointSize(3);
        glBegin(GL_POINTS);
        glVertex2fv(pontos[indiceDoVerticeSelecionado].v);
        glEnd();
    }

    if (doubleBuffer)
        glutSwapBuffers();
    else
        glFlush();
}

void procegVertMenuEvents(int option)
{
    switch (option)
    {
    case 1:
    {
        tipoDoPoligono = GL_LINE_LOOP;
        poligonoJaDesenhado = 1;
        break;
    }
    case 2:
        init();
        break;
    }
    glutPostRedisplay();
}

void subMenuEvents(int option)
{
    operacaoSelecionada = option;
    glutPostRedisplay();
}

void createGLUTMenus()
{
    int menu, submenu;

    submenu = glutCreateMenu(subMenuEvents);
    glutAddMenuEntry("Translation", 1);
    glutAddMenuEntry("Rotation", 2);
    glutAddMenuEntry("Scale", 3);
    glutAddMenuEntry("Shear", 4);
    glutAddMenuEntry("Reflection", 5);

    menu = glutCreateMenu(procegVertMenuEvents);
    glutAddMenuEntry("Limpar", 2);
    glutAddMenuEntry("Gerar poligno", 1);

    glutAddSubMenu("Transformation", submenu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void DefinirMatrizIdentidade(void)
{
    matrizDeBase[0][0] = 1.0f;
    matrizDeBase[0][1] = 0.0f;
    matrizDeBase[0][2] = 0.0f;
    matrizDeBase[1][0] = 0.0f;
    matrizDeBase[1][1] = 1.0f;
    matrizDeBase[1][2] = 0.0f;
    matrizDeBase[2][0] = 0.0f;
    matrizDeBase[2][1] = 0.0f;
    matrizDeBase[2][2] = 1.0f;
}

void CalcularCentro(float cc[])
{
    int i;
    cc[0] = cc[1] = cc[2] = 0.0f;
    for (i = 0; i < numeroDePontosCriados; i++)
    {
        cc[0] += pontos[i].v[0];
        cc[1] += pontos[i].v[1];
        cc[2] += pontos[i].v[2];
    }
    cc[0] /= numeroDePontosCriados;
    cc[1] /= numeroDePontosCriados;
    cc[2] /= numeroDePontosCriados;
}

void TransformarNoPonto(int position)
{
    float pontoTemporario[3];
    int i, j;

    for (i = 0; i < 3; i++)
    {
        pontoTemporario[i] = 0.0f;
        for (j = 0; j < 3; j++)
            pontoTemporario[i] = pontoTemporario[i] + matrizDeBase[i][j] * pontos[position].v[j];
    }

    for (i = 0; i < 3; i++)
        pontos[position].v[i] = pontoTemporario[i];
}

void TransladarPonto(float dx, float dy)
{
    DefinirMatrizIdentidade();

    matrizDeBase[0][2] = dx;
    matrizDeBase[1][2] = dy;

    for (int i = 0; i < numeroDePontosCriados; i++)
        TransformarNoPonto(i);
}

void Rotacionar(float dx, float dy)
{
    int i;
    float oo, teta, vc[3];

    CalcularCentro(vc);
    TransladarPonto(-1 * vc[0], -1 * vc[1]);

    oo = pontos[indiceDoVerticeSelecionado].v[1] * dx - pontos[indiceDoVerticeSelecionado].v[0] * dy;
    teta = anguloDeRotacao;
    if (oo > 0.0f)
        teta = -1.0f * anguloDeRotacao;

    DefinirMatrizIdentidade();

    matrizDeBase[0][0] = cos(teta);
    matrizDeBase[0][1] = -sin(teta);
    matrizDeBase[1][0] = sin(teta);
    matrizDeBase[1][1] = cos(teta);

    for (i = 0; i < numeroDePontosCriados; i++)
        TransformarNoPonto(i);

    TransladarPonto(vc[0], vc[1]);
}

void Escalar(float dx, float dy)
{

    int i;
    float vc[3], Sx, Sy;

    CalcularCentro(vc);
    TransladarPonto(-1 * vc[0], -1 * vc[1]);

    Sx = Sy = 1.0f;
    if (fabs(pontos[indiceDoVerticeSelecionado].v[0]) > 0.01f)
        Sx = 1.0f + dx / pontos[indiceDoVerticeSelecionado].v[0];

    if (fabs(pontos[indiceDoVerticeSelecionado].v[1]) > 0.01f)
        Sy = 1.0f + dy / pontos[indiceDoVerticeSelecionado].v[1];

    DefinirMatrizIdentidade();
    matrizDeBase[0][0] = Sx;
    matrizDeBase[1][1] = Sy;

    for (i = 0; i < numeroDePontosCriados; i++)
        TransformarNoPonto(i);

    TransladarPonto(vc[0], vc[1]);
}

void Cisalhar(float dx, float dy)
{
    int i;
    float Sx, Sy, vc[3];

    CalcularCentro(vc);
    TransladarPonto(-1 * vc[0], -1 * vc[1]);

    Sx = 0;
    Sy = 0;
    if (dx > dy)
    {
        if (fabs(pontos[indiceDoVerticeSelecionado].v[0]) > 0.1f)
            Sx = (dx / pontos[indiceDoVerticeSelecionado].v[1]) / 5;
    }
    if (dy > dx)
    {
        if (fabs(pontos[indiceDoVerticeSelecionado].v[1]) > 0.1f)
            Sy = (dy / pontos[indiceDoVerticeSelecionado].v[0]) / 5;
    }

    DefinirMatrizIdentidade();
    matrizDeBase[0][1] = Sx;
    matrizDeBase[1][0] = Sy;

    for (i = 0; i < numeroDePontosCriados; i++)
        TransformarNoPonto(i);

    TransladarPonto(vc[0], vc[1]);
}

void Refletir(float dx, float dy)
{

    int i;
    float vc[3];

    CalcularCentro(vc);
    TransladarPonto(-1 * vc[0], -1 * vc[1]);
    DefinirMatrizIdentidade();

    if (fabs(dx) > fabs(dy))
    {
        matrizDeBase[0][0] = -1;
        matrizDeBase[1][1] = 1;
    }
    if (fabs(dy) > fabs(dx))
    {
        matrizDeBase[0][0] = 1;
        matrizDeBase[1][1] = -1;
    }

    for (i = 0; i < numeroDePontosCriados; i++)
        TransformarNoPonto(i);

    TransladarPonto(vc[0], vc[1]);
}

void FuncaoDeMovimentacao(int x, int y)
{
    int i;
    float dx, dy;
    if (indiceDoVerticeSelecionado > -1)
    {
        x = x - tamanhoDaJanelaX;
        y = tamanhoDaJanelaY - y;
        dx = x - pontos[indiceDoVerticeSelecionado].v[0];
        dy = y - pontos[indiceDoVerticeSelecionado].v[1];
        switch (operacaoSelecionada)
        {
        case 1:
            TransladarPonto(dx, dy);
            break;
        case 2:
            Rotacionar(dx, dy);
            break;
        case 3:
            Escalar(dx, dy);
            break;
        case 4:
            Cisalhar(dx, dy);
            break;
        case 5:
            Refletir(x, y);
            break;
        }
        DesenharTela();
    }
}

void Mouse(int button, int state, int x, int y)
{

    if (state == GLUT_UP)
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            if (poligonoJaDesenhado == 0)
            {
                x = x - tamanhoDaJanelaX;
                y = tamanhoDaJanelaY - y;

                glPointSize(3);

                pontos[numeroDePontosCriados].v[0] = (float)x;
                pontos[numeroDePontosCriados].v[1] = (float)y;
                pontos[numeroDePontosCriados].v[2] = 1.0f;
                numeroDePontosCriados++;
                indiceDoVerticeSelecionado = (SelecionarVertice(x, y));
            }
        }
        else if (button == GLUT_RIGHT_BUTTON)
        {
            if (numeroDeVertices > 0)
            {
                poligonoJaDesenhado = 1;
                tipoDoPoligono = GL_LINE;
            }
        }
    }
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    GLenum type;

    glutInit(&argc, argv);

    type = GLUT_RGB;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;

    glutInitDisplayMode(type);
    glutInitWindowSize(1200, 800);
    glutCreateWindow("Basic Program Using Glut and Gl");

    init();

    glutReshapeFunc(Reshape);
    glutDisplayFunc(DesenharTela);

    glutMotionFunc(FuncaoDeMovimentacao);
    glutMouseFunc(Mouse);

    createGLUTMenus();

    glutMainLoop();

    return (0);
}