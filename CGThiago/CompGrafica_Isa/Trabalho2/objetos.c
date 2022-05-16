// This is a sample OpenGL/CLUT program for SC208 course.
// Developed by Alexei Sourin, October 2003.
// The students are to study this code and use it 
// as a template for their assignment.
// Open a new project "Win32 Console Application" and add
// sample.c to Source Files 




// The unit size used in this application's World Coordinate System. 
// All other dimensions and parameters are made proportional to it.
#define	SIZE	10.0


#include <GL/glut.h>
#include <stdlib.h> 
#include <stdio.h>   
#include <math.h>

int RendMode; /* May take values 1 or 2: for wireframe and polygon modes. */
double alpha, beta;  /* Angles of rotation about vertical and horizontal axes. */

float local_scale = 1.0f;

typedef float f4d[4];

typedef struct st_vector_int
{
	int n;			// numero de elementos
	int *indice;	// elemento - indice del vertice
	f4d vNormal;    // vetor normal da face
} vector_int;

typedef struct st_vector_f4d
{
	int n;				// numero de vertices
	f4d *vPoint;	// coordenadas de vertice
} vector_f4d;

typedef struct st_vector_faces
{
	int n;						// numero de faces
	vector_int *faceVertice;	// faces por indice vertices
} vector_faces;

typedef struct st_un_objeto
{
	vector_f4d *vertices;		// vector de vertices
	vector_faces *faces;		// vector de faces
	f4d cor; 
	f4d posicao;
	f4d orientacao;
	float escala;	
} un_objeto;

typedef struct st_objetos
{
	int n;                      // numero de objetos
	un_objeto **objs;
} stObjetos;                      // n objetos

// ===========================================

stObjetos *objetos = NULL;		// un objeto

// ===========================================

float mat_ambiente[] = {0.3, 0.0, 0.0, 1.0};
float mat_difuso[] = {0.5, 0.0, 0.0, 1.0};
float mat_especular[] = {1.0, 1.0, 1.0, 1.0};
float mat_emissao[] = {0, 0, 0, 1};
float mat_brilho[] = {50.0};

// Defining a point light source parameters
GLfloat light_position1[] = {   0.0,   10.0*SIZE, 100.0*SIZE, 1.0 };
GLfloat light_ambient[]   = {   0.1,   0.1,   0.1, 1.0 };
GLfloat light_diffuse[]   = {   1.0,   1.0,   1.0, 1.0 };
GLfloat light_specular[]  = {   1.0,   1.0,   1.0, 1.0 };

//============================================================
static void Init(void)
{
// This function is called only once before any other functions are called
	alpha=-20.0; beta=20.0; RendMode=1;


// Setting up a point light source
	glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position1);

// Enabling lighting with the light source #1
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);

// enabling both side illumination for the polygons and hidden surface/line removal
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
}


//============================================================
static void reshape( int width, int height )
{

// This function is called every time the window is to be resized or exposed
    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode (GL_PROJECTION);	
    glLoadIdentity ();
    glFrustum(-3*SIZE, 3*SIZE, -3*SIZE, 3*SIZE, 3*SIZE, 40*SIZE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity ();
}

//============================================================
void calculaNormalFace(vector_int *fac, vector_f4d *vert)
{
	f4d a, b, vNorm;
	float s;
	int nn, io, ia, ib;

	nn = fac->n - 1;
	io = fac->indice[0];
	ib = fac->indice[nn];
	ia = fac->indice[1];

	a[0] = vert->vPoint[ia][0] - vert->vPoint[io][0];
	a[1] = vert->vPoint[ia][1] - vert->vPoint[io][1];
	a[2] = vert->vPoint[ia][2] - vert->vPoint[io][2];

	b[0] = vert->vPoint[ib][0] - vert->vPoint[io][0];
	b[1] = vert->vPoint[ib][1] - vert->vPoint[io][1];
	b[2] = vert->vPoint[ib][2] - vert->vPoint[io][2];
	
	vNorm[0] = a[1]*b[2] - a[2]*b[1];
	vNorm[1] = a[2]*b[0] - a[0]*b[2];
	vNorm[2] = a[0]*b[1] - a[1]*b[0];

	s = sqrt(vNorm[0]*vNorm[0]+vNorm[1]*vNorm[1]+vNorm[2]*vNorm[2]);

	fac->vNormal[0] = vNorm[0] / s;
	fac->vNormal[1] = vNorm[1] / s;
	fac->vNormal[2] = vNorm[2] / s;
}

void DesenhaObjeto(un_objeto *obj)
{

	int i, j, h;

	if(!obj)
		return;

	//glPolygonMode(GL_FRONT_AND_BACK, RendMode);
	//glColor3f(0.0f, 0.0f, 1.0f);
	if (RendMode==1){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);	
		glColor4f(obj->cor[0],obj->cor[1],obj->cor[2],obj->cor[3]);
	}
	else {
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
		glColor4f(obj->cor[0],obj->cor[1],obj->cor[2],obj->cor[3]);
	}
	

	for(i=0; i<obj->faces->n; i++)
	{
		glBegin(GL_POLYGON);
		   	if(RendMode>1) {
		   		calculaNormalFace(&(obj->faces->faceVertice[i]), obj->vertices); // computa normal da face
				glNormal3fv(obj->faces->faceVertice[i].vNormal);
		   	}
			for(j=0; j<obj->faces->faceVertice[i].n; j++)
			{
				h = obj->faces->faceVertice[i].indice[j];
				glVertex3fv(obj->vertices->vPoint[h]);
			}
		glEnd();	
	}


} 

void MakeShapes(void){
	
	glDisable(GL_LIGHTING);
    glBegin(GL_LINES);

	glColor4f(1.0, 0.0, 0.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(3*SIZE,0.0,0.0);

	glColor4f(0.0, 1.0, 0.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,3*SIZE,0.0);

	glColor4f(0.0, 0.0, 1.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,0.0,3*SIZE);
	
	   
	glEnd();


	//if(objeto) DesenhaObjeto(objeto);
}

 

un_objeto* liberaObjeto(un_objeto* obj)
{
	int i, j;

	if(obj)
	{
		if(obj->vertices)
		{
			if(obj->vertices->vPoint)
				free(obj->vertices->vPoint);
			free(obj->vertices);
		}
		if(obj->faces)
		{
			if(obj->faces->faceVertice)
			{
				for(j=0; j<obj->faces->n; j++)
					free(obj->faces->faceVertice[j].indice);

				free(obj->faces->faceVertice);
			}
			free(obj->faces);
		}

		free(obj);
	}
	return NULL;
}

stObjetos  *liberaObjetos(stObjetos* objetos){
	int i;
	for(i=0; i<objetos->n; i++){
		liberaObjeto(objetos->objs[i]);
		objetos->objs[i] = NULL;
	}
	free(objetos->objs);
	free(objetos);

	return NULL;
}

un_objeto *procSolido(char *arch)
{
  FILE *fobj;
  char token[40];
  float px, py, pz;
  int i, j, n, m;
  un_objeto *objeto;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0; 
  }
   
  //if (objeto) objeto = liberaObjeto(objeto);

  if((objeto = (un_objeto*) malloc(sizeof(un_objeto)))==NULL)
  {
		printf("\n Error en alocacion de memoria para un_objeto");
		return 0;
  }

  fgets(token, 40, fobj);  /* #cabecalho */ 
  fscanf(fobj,"%s %d", token, &n);

  objeto->vertices = (vector_f4d*) malloc(sizeof(vector_f4d));

  objeto->vertices->n = n;
  objeto->vertices->vPoint = (f4d*) calloc(n, sizeof(f4d));
  for(j=0; j<objeto->vertices->n; j++)
  {
	 fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);
	 
     objeto->vertices->vPoint[j][0] = px * local_scale;     
     objeto->vertices->vPoint[j][1] = py * local_scale;
     objeto->vertices->vPoint[j][2] = pz * local_scale;     
	 objeto->vertices->vPoint[j][3] = 0.0f;
  }

  fscanf(fobj,"%s %i", token, &n);

  objeto->faces = (vector_faces*) malloc(sizeof(vector_faces));

  objeto->faces->n = n;
  objeto->faces->faceVertice = (vector_int*) calloc(n, sizeof(vector_int));
  for(j=0; j<objeto->faces->n; j++)
  {
	 fscanf(fobj, "%s %i", token, &m);
	 objeto->faces->faceVertice[j].n = m;
	 objeto->faces->faceVertice[j].indice = (int*) calloc(m, sizeof(int));

	 for(i=0; i<objeto->faces->faceVertice[j].n; i++)
	 {
		fscanf(fobj, "%i", &m);
		objeto->faces->faceVertice[j].indice[i] = m;
	 }
  }

  fclose(fobj);
  return objeto;
}

int procObjetos(char *arch)
{
  FILE *fobj;
  char token[40], Arquivo[40];
  int i, n;
  float x, y, z, d, s;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0; 
  }
   
  if (objetos) objetos = liberaObjetos(objetos);

  if((objetos = (stObjetos*) malloc(sizeof(stObjetos)))==NULL)
  {
		printf("\n Error en alocacion de memoria para un_objeto");
		return 0;
  }

  fgets(token, 40, fobj);

  fscanf (fobj, "%s %d", token, &n);

  objetos->objs = (un_objeto**) malloc(n*sizeof(un_objeto*));
  objetos->n = n;
    
  for(i=0; i<objetos->n; i++){
  	fscanf(fobj, "%s %s", token, Arquivo);
  	objetos->objs[i] = procSolido(Arquivo);

  	fscanf(fobj, "%s %f %f %f %f", token, &x, &y, &z, &d);
  	objetos->objs[i]->cor[0] = x;
  	objetos->objs[i]->cor[1] = y;
  	objetos->objs[i]->cor[2] = z;
  	objetos->objs[i]->cor[3] = d;

  	fscanf(fobj, "%s %f %f %f", token, &x, &y, &z);
  	objetos->objs[i]->posicao[0] = x;
  	objetos->objs[i]->posicao[1] = y;
  	objetos->objs[i]->posicao[2] = z;
  	objetos->objs[i]->posicao[3] = 0.0;

  	fscanf(fobj, "%s %f %f %f", token, &x, &y, &z);
  	objetos->objs[i]->orientacao[0] = x;
  	objetos->objs[i]->orientacao[1] = y;
  	objetos->objs[i]->orientacao[2] = z;
  	objetos->objs[i]->orientacao[3] = 0.0;

  	fscanf(fobj, "%s %f", token, &s);
  	objetos->objs[i]->escala = s;
  }
   
  fclose(fobj);
  return 1;

}

void processMenuEvents(int option) 
{
	if (option == 1)
		procObjetos("data/umObjeto1.txt");
	else if (option == 2)
		procObjetos("data/umObjeto2.txt");
	else if (option == 3)
		procObjetos("data/Objetos.txt");
	/*else if(option == 4)
		RendMode = GL_LINE;
	else if (option == 5)
		RendMode = GL_FILL;*/

	glutPostRedisplay();
}

//============================================================
static void draw( void ){

	int i;

	glClearColor (0.0, 0.0, 0.2, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
	glMatrixMode(GL_MODELVIEW);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambiente);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_difuso);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_especular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_brilho);
	glEnable(GL_COLOR_MATERIAL);
	glPushMatrix();
// Placement and rotation of the scene.

	if(objetos)
	for(i=0; i<objetos->n; i++){
	glTranslatef(objetos->objs[i]->posicao[0], objetos->objs[i]->posicao[1], objetos->objs[i]->posicao[2]);
	glRotatef(beta, 1.0, 0.0, 0.0);
	glRotatef(alpha, 0.0, 1.0, 0.0);
	MakeShapes();
	DesenhaObjeto(objetos->objs[i]);
 		
	}

	glPopMatrix();    
		
// This command will swap animation buffers to display the current frame.
    glutSwapBuffers();
}

//============================================================
static void idle( void )
{
// This function will call draw() as frequent as possible thus enabling us to make 
// interaction and animation.

   draw();
}

static  void hotkey(unsigned char k, int x, int y)
{
// Here we are processing keyboard events.
   switch (k) 
   {
      case 27:
	  exit (0);
	  break;

// Switch to wireframe rendering
	  case 'w':
	  RendMode=1;
      break;

// Switch to flat shading
	  case 'f':
	  RendMode=2;
	  glShadeModel(GL_FLAT);
	  break;		  
   }
   glutPostRedisplay();
}

static void sfunc(int k, int x, int y)
{
// Here we can process function keys and other special key events
   switch (k) 
   {

//Rotate to the top
   	  case GLUT_KEY_UP:
   	  beta+=3.0;
   	  break;

//Rotate to the top
   	  case GLUT_KEY_DOWN:
   	  beta-=3.0;
   	  break;


// Rotate to the left
	  case GLUT_KEY_LEFT:
	  alpha-=3.0;
	  break;

// Rotate to the right
	  case GLUT_KEY_RIGHT:
	  alpha+=3.0;
	  break;
   }
   glutPostRedisplay();
}

void createGLUTMenus() 
{
	int menu, submenu1, submenu2;
	
	submenu1 = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Objeto 1",1);
	glutAddMenuEntry("Objeto 2",2);
	glutAddMenuEntry("Objetos 1 e 2",3);

	/*submenu2 = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Aramado",4);
	glutAddMenuEntry("Realismo",5);*/

	menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Tipo de Objeto",submenu1);
	glutAddSubMenu("Exibe objeto",submenu2);

	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

//============================================================
void main( int argc, char *argv[] )
{

// This is the main program where glut functions are invoked to set up 
// the graphics window as well as to define the callback functions.
   glutInit(&argc, argv);
   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB );
   glutInitWindowSize (650, 650); 
   glutInitWindowPosition (100, 100);
   glutCreateWindow ("Objetos 3D - Isabela Correia");

   Init();
   glutReshapeFunc(reshape);
  // glutIdleFunc(idle);
   glutDisplayFunc(draw);
   glutKeyboardFunc(hotkey);
   glutSpecialFunc(sfunc);

// Refer to GLUT manual on how to set up and use pop-up menus.

// The main event loop is started here.
   createGLUTMenus(); 
   glutMainLoop();
}