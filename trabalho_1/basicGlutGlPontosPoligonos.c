#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "GL/glut.h" 

#define MAXVERTEXS 30

#define PHI 3.141572
 
GLenum doubleBuffer;     
 
typedef struct spts 
{ 
    float v[3]; 
} tipoPto; 

//tipoPto ppuntos[MAXVERTEXS];

tipoPto pvertex[MAXVERTEXS];

int windW, windH;
int tipoPoligono;
int nVertices = 0;
int jaPoligono = 0;

void circulo(float r, float ang, float pp[3])
{
	pp[0] = (float)(r * cos(ang));
	pp[1] = (float)(r * sin(ang));
	pp[2] = (float)0.0;
}

void init(void)
{
	int i;

	jaPoligono = 0;
	nVertices=0;   // zero pontos
	tipoPoligono = GL_POINTS;

	for(i=0; i<MAXVERTEXS; i++)
	{
		pvertex[i].v[0] = 0.0f;
		pvertex[i].v[1] = 0.0f;
		pvertex[i].v[2] = 0.0f;
	}
}

static void Reshape(int width, int height)
{
    windW = width/2;
    windH = height/2;

    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
//    gluPerspective(60.0, 1.0, 0.1, 1000.0);
    gluOrtho2D(-windW, windW, -windH, windH);

    glMatrixMode(GL_MODELVIEW);
}

static void Key(unsigned char key, int x, int y)
{
    switch (key) 
	{
      case 27:
			exit(0);
    }
}

void coord_line(void)
{
    glLineWidth(1);

	glColor3f(1.0, 0.0, 0.0);

	// vertical line

	glBegin(GL_LINE_STRIP);
		glVertex2f(-windW, 0);
		glVertex2f(windW, 0);
    glEnd();

		glColor3f(0.0, 1.0, 0.0);

	// horizontal line 

    glBegin(GL_LINE_STRIP);
		glVertex2f(0, -windH);
		glVertex2f(0, windH);
    glEnd();
}

void PolygonDraw(void)
{
	int i;

	glColor3f(0.0, 0.0, 0.0); 

	glPolygonMode(GL_FRONT_AND_BACK, tipoPoligono);

	glBegin(tipoPoligono);
	for(i=0; i<nVertices; i++)
	{
		glVertex2fv(pvertex[i].v);
	}
	glEnd();
}

static void Draw(void)
{
    glClearColor(1.0, 1.0, 1.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);

	coord_line();

	PolygonDraw();

    if (doubleBuffer) 
	{
	   glutSwapBuffers(); 
    } else {
	   glFlush();     
    }
}

void processMenuEvents(int option) 
{
	switch (option) 
	{
		case 2 : 
			init();
			break;
	}
	glutPostRedisplay();
}

void processSubMenuEvents(int option) 
{
	if(option == 0)
		tipoPoligono = GL_POINTS;
	else
		if (option == 1)
		{
			tipoPoligono = GL_LINE_LOOP;
			jaPoligono = 1;
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
	   } else if (strcmp(argv[i], "-db") == 0) 
	   {
	      doubleBuffer = GL_TRUE;
	   }
    }
}

void createGLUTMenus() 
{
	int menu,submenu;

	submenu = glutCreateMenu(processSubMenuEvents);
	glutAddMenuEntry("Pontos",0);
	glutAddMenuEntry("Poligono",1);

	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Limpar",2);
	glutAddSubMenu("Tipo Objeto",submenu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void motion(int x, int y)
{

}

void mouse(int button, int state, int x, int y)
{ 

	if(state == GLUT_UP)
	{
        printf("\n jaPoligono %d ", jaPoligono);
		if(button == GLUT_LEFT_BUTTON) 
		{
			if(jaPoligono==0)
			{
				x = x - windW; 
				y = windH - y;

//				glColor3f(0.0, 1.0, 0.0);
				glPointSize(3);
//				glBegin(GL_POINTS); 
//				glVertex2i(x, y);
//				glEnd();

				pvertex[nVertices].v[0] = (float)x;
				pvertex[nVertices].v[1] = (float)y;
				nVertices++;
			}
			else
			{
                
            }
		} 
		else 
		if(button == GLUT_RIGHT_BUTTON)
		{
			if(nVertices>0)
			{
				jaPoligono = 1;
				tipoPoligono = GL_LINE;
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

	init();

    glutReshapeFunc(Reshape);
    glutKeyboardFunc(Key);
    glutDisplayFunc(Draw);

	glutMotionFunc(motion);
	glutMouseFunc(mouse);
//	glutIdleFunc(idle);


	createGLUTMenus();  

    glutMainLoop();

	return (0);
}
