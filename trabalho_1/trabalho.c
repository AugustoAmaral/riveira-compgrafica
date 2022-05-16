#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

GLenum doubleBuffer;

#define LARGURA_TELA_X 800 // largura da tela
#define ALTURA_TELA_Y 600  // altura da tela
#define TITULO_TELA "Trabalho do riveira"
#define COR_DE_FUNDO 1.0, 1.0, 1.0, 0.0

#define GROSSURA_DA_LINHA_DO_EIXO 1
#define COR_DA_LINHA 1.0, 0.0, 0.0
#define COR_DO_PONTO 0.0, 0.0, 0.0

#define TAMANHO_DO_PONTO 5
#define MAXIMO_DE_PONTOS 10

// Util para seleção de ponto
#define LIMITE_DISTANCIA_MOUSE_VERTICE 10.0
#define COR_DA_VERTICE_SELECIONADA 0.0, 1.0, 0.0
#define TAMANHO_FEEDBACK_VERTICE_SELECIONADA 5

// Escopo de variaveis globais

struct ponto
{
    float x;
    float y;
    float z;
} pontos[MAXIMO_DE_PONTOS];

// GL_POINTS = Pontos | GL_LINE_LOOP = Poligono
int modoDoDesenho = GL_POINTS;

int quantidadeDePontos = 0;
int operacaoSelecionada = 0;
int verticeSelecionado = -1;

// Funçao que redesenha o polígono dentro da tela
void Reshape(void)
{
    glViewport(0, 0, LARGURA_TELA_X, ALTURA_TELA_Y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-LARGURA_TELA_X / 2, LARGURA_TELA_X / 2, -ALTURA_TELA_Y / 2, ALTURA_TELA_Y / 2);
    glMatrixMode(GL_MODELVIEW);
}

float *GerarVetorDePontos(float x, float y, float z)
{
    float *vetor = (float *)malloc(3 * sizeof(float));
    vetor[0] = x;
    vetor[1] = y;
    vetor[2] = z;
    return vetor;
}

void DesenharPoligono(void)
{
    int vertice;

    glColor3f(COR_DO_PONTO);
    glPolygonMode(GL_FRONT_AND_BACK, modoDoDesenho);

    glBegin(modoDoDesenho);
    for (vertice = 0; vertice < quantidadeDePontos; vertice++)
    {
        glVertex2fv(GerarVetorDePontos(pontos[vertice].x, pontos[vertice].y, pontos[vertice].z));
    }
    glEnd();
}

void DesenharEixos(void)
{
    // Grossura da linha do eixo
    glLineWidth(GROSSURA_DA_LINHA_DO_EIXO);
    // Cor da linha do eixo
    glColor3f(COR_DA_LINHA);

    // Desenha o eixo X
    glBegin(GL_LINE_STRIP);
    glVertex2f(-LARGURA_TELA_X, 0);
    glVertex2f(LARGURA_TELA_X, 0);
    glEnd();

    // Desenha o eixo Y
    glBegin(GL_LINE_STRIP);
    glVertex2f(0, -ALTURA_TELA_Y);
    glVertex2f(0, ALTURA_TELA_Y);
    glEnd();
}

void DesenharPontoNoVertice(void)
{
    glColor3f(COR_DA_VERTICE_SELECIONADA);
    glPointSize(TAMANHO_FEEDBACK_VERTICE_SELECIONADA);
    glBegin(GL_POINTS);
    glVertex2fv(GerarVetorDePontos(pontos[verticeSelecionado].x, pontos[verticeSelecionado].y, pontos[verticeSelecionado].z));
}

void PreencherTela(void)
{
    glClearColor(COR_DE_FUNDO);
    glClear(GL_COLOR_BUFFER_BIT);

    DesenharEixos();
    DesenharPoligono();
    if (verticeSelecionado > -1)
        DesenharPontoNoVertice();

    if (doubleBuffer)
        glutSwapBuffers();
    else
        glFlush();
}

void InicializarMenu(void){};

void MenuDesenhos(int opcao)
{
    switch (opcao)
    {
    case 0:
        modoDoDesenho = GL_POINTS;
        break;
    case 1:
        modoDoDesenho = GL_LINE_LOOP;
        break;
    case 2:
        modoDoDesenho = GL_POINTS;
        quantidadeDePontos = 0;
        break;
    }
    glutPostRedisplay();
};

void MenuOperacoes(int opcao)
{
    if (modoDoDesenho == GL_POINTS)
        operacaoSelecionada = opcao;
};

void CriarMenus(void)
{
    int menuDeDesenho = glutCreateMenu(MenuDesenhos);
    glutAddMenuEntry("Fazer pontos", 0);
    glutAddMenuEntry("Desenhar poligono", 1);
    glutAddMenuEntry("Limpar pontos", 2);

    int menuDeOperacoes = glutCreateMenu(MenuOperacoes);
    glutAddMenuEntry("Deselecionar", 0);
    glutAddMenuEntry("Mover", 1);
    glutAddMenuEntry("Rotacionar", 2);
    glutAddMenuEntry("Escalar", 3);
    glutAddMenuEntry("Cisalhar", 4);

    glutCreateMenu(InicializarMenu);
    glutAddMenuEntry("Menu de operacoes", 0);
    glutAddSubMenu("Desenhar Objeto", menuDeDesenho);
    glutAddSubMenu("Transformar", menuDeOperacoes);

    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int SelecionarVertice(int posX, int posY)
{
    printf("Estou aqui %d %d %d\n", posX, posY, verticeSelecionado);
    int vertice;     // Vertice a ser checada
    float distancia; // valor da distancia calculada entre o clique e o vertice
    verticeSelecionado = -1;

    for (vertice = 0; vertice < quantidadeDePontos; vertice++)
    {
        distancia = sqrt(pow((pontos[vertice].x - posX), 2.0) + pow((pontos[vertice].y - posY), 2.0));
        if (distancia < LIMITE_DISTANCIA_MOUSE_VERTICE)
        {
            verticeSelecionado = vertice;
            break;
        }
    }
}
/**
 * @brief Função responsavel para realizar interações do mouse
 * Caso poligono não esteja desenhado, adicionar ponto na tela
 * Caso contrario, selecionar vertice proxima caso exista
 *
 * @param button
 * @param state
 * @param x
 * @param y
 */
void Mouse(int button, int state, int x, int y)
{
    if (state == GLUT_UP)
    {
        if (button == GLUT_LEFT_BUTTON)
        {
            float posX = (float)(x) - (LARGURA_TELA_X / 2);
            float posY = (ALTURA_TELA_Y / 2) - (float)(y);
            if (modoDoDesenho == GL_POINTS)
            {
                glPointSize(TAMANHO_DO_PONTO);
                pontos[quantidadeDePontos].x = posX;
                pontos[quantidadeDePontos].y = posY;
                quantidadeDePontos++;
            }
            else
            {
                // Resetar o vertice selecionado
                SelecionarVertice(posX, posY);
            }
        }
    }
    PreencherTela();
}

int main(int argc, char **argv)
{
    // Inicia o glut
    GLenum type;
    glutInit(&argc, argv);
    type = GLUT_RGB;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
    glutInitDisplayMode(type);
    // Cria a tela
    glutInitWindowSize(LARGURA_TELA_X, ALTURA_TELA_Y);
    // Define o título da tela
    glutCreateWindow(TITULO_TELA);
    // Remodela a tela para desenhar pelo meio da tela
    glutReshapeFunc(Reshape);
    // Cria o desenho inicial na tela
    glutDisplayFunc(PreencherTela);

    glutMouseFunc(Mouse);
    // Criar os menus do gluth
    CriarMenus();

    glutMainLoop();
    return 0;
}