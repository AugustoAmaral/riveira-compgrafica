#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h"

GLenum doubleBuffer;

#define LARGURA_TELA_X 800         // largura da tela
#define ALTURA_TELA_Y 600          // altura da tela
#define TITULO_TELA "Trabalho do riveira"
#define COR_DE_FUNDO 1.0, 1.0, 1.0, 0.0

// Funçao que redesenha o polígono dentro da tela
void Reshape(void)
{
    glViewport(0, 0, LARGURA_TELA_X, ALTURA_TELA_Y);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-LARGURA_TELA_X/2, LARGURA_TELA_X/2, -ALTURA_TELA_Y/2, ALTURA_TELA_Y/2);
    glMatrixMode(GL_MODELVIEW);
}

static void PreencherTela(void)
{
    glClearColor(COR_DE_FUNDO);
    glClear(GL_COLOR_BUFFER_BIT);

    if (doubleBuffer)
        glutSwapBuffers();
    else
        glFlush();
}

int main(int argc, char **argv)
{
    // Inicia o glut
    GLenum type;
    glutInit(&argc, argv);
    type = GLUT_RGB;
    type |= (doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE;
    glutInitDisplayMode(type);
    //Cria a tela
    glutInitWindowSize(LARGURA_TELA_X, ALTURA_TELA_Y);
    // Define o título da tela
    glutCreateWindow(TITULO_TELA);
    // Remodela a tela para desenhar pelo meio da tela
    glutReshapeFunc(Reshape);
    // Cria o desenho inicial na tela
    glutDisplayFunc(PreencherTela);

    glutMainLoop();
    return 0;
}