// ------------------------------------------------------------------------------------
// Trabalho: 
//        - Geracao de superficie aberto com maior numero de patchs e formas variadas
//        - Pode adicionar operacoes: Escala, Rotacao, Translacao
//        - Uso de Bases BSplines e CatmullRom
//        - Eficiencia de realismo com triangulacao eficiente
//        - Combinacao de cores e tonalidades
//        - Melhoria de controle de manupilacao (mouse? ao inves de teclado?)
// ------------------------------------------------------------------------------------
// Atender as 4 OBSERVACOES ....
// ------------------------------------------------------------------------------------
// Data de entrega e apresentacao: at� a 2da prova
// Trabalho individual (similares Uma nota dividda em numeo de similares)
// -------------------------------------------------------------------------------------                 

#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>



#define Linha -1
#define Solido -2 
#define Pontos -3
#define PtsControle -4
#define X 0
#define Y 1
#define Z 2
#define W 3
#define Escalar 4 
#define EscalarX 5
#define EscalarY 6
#define EscalarZ 7
#define RotarX 8
#define RotarY 9
#define RotarZ 10
#define TransladarX 11
#define TransladarY 12
#define BEZIER      20
#define BSPLINE     21
#define CATMULLROM  22

#define sair 0

typedef float f4d[4];

typedef struct st_matriz
{
    int n, m;
    f4d **ponto;
} matriz;
 
int comando = RotarX; 

int tipoView = GL_LINE_STRIP;

float local_scale = 0.22f;

float VARIA = 0.01f;


f4d matTransf[4];

f4d MatBase[4];   // matriz de base de qualquer tipo da curva

f4d pView = {10.0, 10.0, -20.0, 0.0};

	// ---------------------------------------------
	// OBSERVACAO 1: cores
	//       definir mais cores 
    // ---------------------------------------------
f4d vcolor[9] = {{1.0, 0.0, 0.0, 0.0},
				 {0.0, 1.0, 0.0, 0.0},
				 {0.0, 0.0, 1.0, 0.0},
				 {1.0, 1.0, 0.0, 0.0},
                 {1.0, 0.0, 1.0, 0.0},
                 {1.0, 0.0, 0.0, 1.0},
                 {0.0, 1.0, 1.0, 0.0},
                 {0.0, 1.0, 0.0, 1.0},
                 {0.0, 0.0, 1.0, 1.0}};

// -------------------------------------------------------------------
//  S�o requeridos 3 matrizes:
//        1) pcObjeto:  matriz de todos os pontos de controle do objeto 
//                      n x m pontos de controle
//        2) pcPatch:   matriz de pontos de controle para gerar um PATCH
//                      neste caso 4 x 4
//        3) ptsPatch:  matriz de pontos calculados de um patch da superficie
//                      nn x mm (dimensao dependa da resolucao da supercies)
// -------------------------------------------------------------------

matriz *pcObjeto = NULL;    // matriz de pontos de controle - LIDOS do arquivo
matriz *pcPatch = NULL;    // matriz de pontos para um patch - copiados de pcObjeto
matriz *ptsPatch = NULL;   // matriz de PONTOS do PATCH da superficie


void designSurface(void);

// ----------------------------------------------------------------------------
// OBSERVACAO 2:
//     Definir as matrizes BASE 
// ----------------------------------------------------------------------------
void MontaMatrizBase(int tipoSup)
{
	if(tipoSup==BEZIER)
	{
		MatBase[0][0] = -1.0f; MatBase[0][1] = 3.0f;  MatBase[0][2] = -3.0f; MatBase[0][3] = 1.0f;
		MatBase[1][0] =  3.0f; MatBase[1][1] = -6.0f; MatBase[1][2] =  3.0f; MatBase[1][3] = 0.0f;
		MatBase[2][0] = -3.0f; MatBase[2][1] = 3.0f;  MatBase[2][2] =  0.0f; MatBase[2][3] = 0.0f;
		MatBase[3][0] =  1.0f; MatBase[3][1] = 0.0f;  MatBase[3][2] =  0.0f; MatBase[3][3] = 0.0f;
	}
	if(tipoSup==BSPLINE)
    {
        MatBase[0][0] = -1.0/6.0; MatBase[0][1] = 3.0/6.0;  MatBase[0][2] = -3.0/6.0; MatBase[0][3] = 1.0/6.0;
        MatBase[1][0] =  3.0/6.0; MatBase[1][1] = -6.0/6.0; MatBase[1][2] =  3.0/6.0; MatBase[1][3] = 0.0/6.0;
        MatBase[2][0] = -3.0/6.0; MatBase[2][1] = 0.0/6.0;  MatBase[2][2] =  3.0/6.0; MatBase[2][3] = 0.0/6.0;
        MatBase[3][0] =  1.0/6.0; MatBase[3][1] = 4.0/6.0;  MatBase[3][2] =  1.0/6.0; MatBase[3][3] = 0.0/6.0;
    }
    if(tipoSup==CATMULLROM)
    {
        MatBase[0][0] = -1.0/2.0; MatBase[0][1] = 3.0/2.0;  MatBase[0][2] = -3.0/2.0; MatBase[0][3] = 1.0/2.0;
        MatBase[1][0] =  2.0/2.0; MatBase[1][1] = -5.0/2.0; MatBase[1][2] =  4.0/2.0; MatBase[1][3] = -1.0/2.0;
        MatBase[2][0] = -1.0/2.0; MatBase[2][1] = 0.0/2.0;  MatBase[2][2] =  1.0/2.0; MatBase[2][3] = 0.0/2.0;
        MatBase[3][0] =  0.0/2.0; MatBase[3][1] = 2.0/2.0;  MatBase[3][2] =  0.0/2.0; MatBase[3][3] = 0.0/2.0;
    }
}

matriz* liberaMatriz(matriz* sup)
{
	int i;

	if(sup)
	{
	    for(i=0; i< sup->n; i++)
	        free(sup->ponto[i]);

	    free(sup->ponto);
		free(sup);
	}
	return NULL;
}


matriz* AlocaMatriz(int n, int m)
{
	matriz *matTemp;
	int j;

    if((matTemp = (matriz*) malloc(sizeof(matriz)))==NULL)
    {
		printf("\n Error en alocacion de memoria para uma matriz");
		return 0;
    }

    matTemp->n = n;
    matTemp->m = m;
    matTemp->ponto = (f4d**) calloc(n, sizeof(f4d*));

    for(j=0; j<matTemp->n; j++)
         matTemp->ponto[j] = (f4d*) calloc(m, sizeof(f4d));

    return matTemp;
}


void MatrizIdentidade()
{
	int a,b;
    for(a=0; a<4; a++)
    {   for(b=0; b<4; b++)
		{   if(a==b)
		       matTransf[a][b] = 1;
		    else
		       matTransf[a][b] = 0;
		}
    }
}

void MultMatriz()
{ 
	int j, k;
    f4d aux;
    for(j=0; j< pcObjeto->n; j++)
	{
		for(k = 0; k< pcObjeto->m; k++)
		{
         aux[X] = pcObjeto->ponto[j][k][X]; 
		 aux[Y] = pcObjeto->ponto[j][k][Y]; 
		 aux[Z] = pcObjeto->ponto[j][k][Z];
		 aux[W] = 1.0; //pcObjeto->ponto[j][k][W];

		 //  Pj = MatTransf4x4 . Pj  <--- transformada homogenea 

         pcObjeto->ponto[j][k][X] = matTransf[X][X] * aux[X] + 
							         matTransf[Y][X] * aux[Y] + 
							         matTransf[Z][X] * aux[Z] +
									 matTransf[W][X] * aux[W];

		 pcObjeto->ponto[j][k][Y] = matTransf[X][Y] * aux[X] + 
							         matTransf[Y][Y] * aux[Y] + 
							         matTransf[Z][Y] * aux[Z] +
									 matTransf[W][Y] * aux[W];

		 pcObjeto->ponto[j][k][Z] = matTransf[X][Z] * aux[X] + 
							         matTransf[Y][Z] * aux[Y] + 
							         matTransf[Z][Z] * aux[Z] +
									 matTransf[W][Z] * aux[W];

		 pcObjeto->ponto[j][k][W] = matTransf[X][W] * aux[X] + 
							         matTransf[Y][W] * aux[Y] + 
							         matTransf[Z][W] * aux[Z] +
									 matTransf[W][W] * aux[W];
		}
	}
}

void prod_VetParam_MatBase(float t, float *tt, float *vr)
{
	int i, j;

	// Vr = [t^3  t^2  t  1] [MatBase]4x4  <-- base avaliado em t.
	// ---------------------------------------------------------
	tt[0] = pow(t,3);
	tt[1] = pow(t,2);
	tt[2] = t;
	tt[3] = 1.0;

	for(i=0; i<4; i++)
	{
		vr[i] = 0.0f;
		for(j=0; j<4; j++)
			vr[i] += MatBase[j][i] * tt[j];
	}
}

void prod_VetMatriz(float *v, f4d **pc, f4d *vr)
{
	int i, j;

	// Vr = V . P   <----- combinacao linear
	// --------------------------------------
	for(i=0; i<4; i++)
	{
		vr[i][0] = vr[i][1] = vr[i][2] = 0.0;
		for(j=0; j<4; j++)
		{
			vr[i][0] += v[j] * pc[j][i][0];
			vr[i][1] += v[j] * pc[j][i][1];
			vr[i][2] += v[j] * pc[j][i][2];
		}	
	}
}

void ptsSuperficie(matriz *pcPatch)
{
    int i, j, h, n, m;
	float t,s;
	float tmp[4], vsm[4], vtm[4];
	f4d va[4];

	if(!pcPatch) return;

    n = 0;

    for(s = 0; s<=1.01; s+=VARIA) n += 1;

    m = n;

    if (ptsPatch) ptsPatch = liberaMatriz(ptsPatch);

    ptsPatch=AlocaMatriz(n,m);

	// Gera uma matriz de ptsPatch de n x m pontos de cada PATCH
	// para  ( 0 <= s <= 1 , 0 <= s <= 1 )
	
	s=0.0f;
	for(i = 0; i < ptsPatch->n; i++)
    {
        t = 0.0f;
        for(j = 0; j < ptsPatch->m; j++)
        {
				// calcula cada ponto: p(s, t) = S G P G^t T

			prod_VetParam_MatBase(s, tmp, vsm);    // vsm = S G
			prod_VetParam_MatBase(t, tmp, vtm);    // vtm = G^t T

			prod_VetMatriz(vsm, pcPatch->ponto, va);    // va = S G P = vsm P

			ptsPatch->ponto[i][j][0] = 0.0f;
			ptsPatch->ponto[i][j][1] = 0.0f;
			ptsPatch->ponto[i][j][2] = 0.0f;

			for(h=0; h<4; h++)						// p = S G P G^t T = va vtm
			{
				ptsPatch->ponto[i][j][0] += va[h][0] * vtm[h];
				ptsPatch->ponto[i][j][1] += va[h][1] * vtm[h];
				ptsPatch->ponto[i][j][2] += va[h][2] * vtm[h];
			}
			t+=VARIA;
        }
        s+=VARIA;
    }
}

void MostrarUmPatch(int cc)
{
	int i, j;
	float t,v,s;
    f4d a,b,n,l;

    if(!ptsPatch)  return;

    switch(tipoView)
    {
        case GL_POINTS:
          glColor3f(0.0f, 0.0f, 0.7f);
          glPointSize(1.0);
          for(i = 0; i < ptsPatch->n; i++)
          {
              glBegin(tipoView);
              for(j = 0; j < ptsPatch->m; j++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }
          break;

        case GL_LINE_STRIP:
          glColor3f(0.0f, 0.0f, 0.7f);
          for(i = 0; i < ptsPatch->n; i++)
          {
              glBegin(tipoView);
              for(j = 0; j < ptsPatch->m; j++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }

          for(j = 0; j < ptsPatch->n; j++)
          {
              glBegin(tipoView);
              for(i = 0; i < ptsPatch->m; i++)
                 glVertex3fv(ptsPatch->ponto[i][j]);
              glEnd();
          }
          break;

        case GL_QUADS:
 	      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
          for(i = 0; i < ptsPatch->n-1; i++)
          {
              for(j = 0; j < ptsPatch->m-1; j++)
              {

	// -----------------------------------------------------
	// OBSERVACAO 3: modificar
	//  Ver a melhor forma de criar triangulos 1ro e 2do
    // -----------------------------------------------------
    
				  // criando 1ro triangulo do quadrilatero

				a[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i][j][X];
				a[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i][j][Y];
				a[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i][j][Z];

				b[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i][j][X];
				b[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i][j][Y];
				b[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i][j][Z];

				n[X] = a[Y]*b[Z] - a[Z]*b[Y];
				n[Y] = a[Z]*b[X] - a[X]*b[Z];
				n[Z] = a[X]*b[Y] - a[Y]*b[X];

				s = sqrt(n[X]*n[X]+n[Y]*n[Y]+n[Z]*n[Z]);

				n[X] /=s; n[Y] /=s; n[Z] /=s;

				l[X] = pView[X] - ptsPatch->ponto[i][j][X];
				l[Y] = pView[Y] - ptsPatch->ponto[i][j][Y];
				l[Z] = pView[Z] - ptsPatch->ponto[i][j][Z];

				s = n[X]*l[X]+n[Y]*l[Y]+n[Z]*l[Z];

				v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
				t = s / v;

				if(t<0.0f)
					t *= -1.00f;

				glBegin(GL_POLYGON);
					glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
					glNormal3fv(n);
                    glVertex3fv(ptsPatch->ponto[i][j]);
                    glVertex3fv(ptsPatch->ponto[i][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j]);
				glEnd();

				// criando 2do triangulo


				a[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i+1][j+1][X];
				a[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i+1][j+1][Y];
				a[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i+1][j+1][Z];

				b[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i+1][j+1][X];
				b[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i+1][j+1][Y];
				b[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i+1][j+1][Z];

				n[X] = a[Y]*b[Z] - a[Z]*b[Y];
				n[Y] = a[Z]*b[X] - a[X]*b[Z];
				n[Z] = a[X]*b[Y] - a[Y]*b[X];

				s = sqrt(n[X]*n[X]+n[Y]*n[Y]+n[Z]*n[Z]);

				n[X] /=s; n[Y] /=s; n[Z] /=s;

				l[X] = pView[X] - ptsPatch->ponto[i+1][j+1][X];
				l[Y] = pView[Y] - ptsPatch->ponto[i+1][j+1][Y];
				l[Z] = pView[Z] - ptsPatch->ponto[i+1][j+1][Z];

				s = n[X]*l[X]+n[Y]*l[Y]+n[Z]*l[Z];

				v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
				t = s / v;

				if(t<0.0f)
					t *= -1.00f;

				glBegin(GL_POLYGON);
					glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
					glNormal3fv(n);
                    glVertex3fv(ptsPatch->ponto[i][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j+1]);
                    glVertex3fv(ptsPatch->ponto[i+1][j]);
				glEnd();
              }
          }
          break;
    }

}

void MostrarPtosPoligControle(matriz *sup)
{
	int i, j;

    glColor3f(0.0f, 0.8f, 0.0f);
	glPolygonMode(GL_FRONT_AND_BACK, tipoView);
	glPointSize(7.0);
	for(i=0; i<sup->n; i++)
	{
		glBegin(GL_POINTS);
		for(j=0; j<sup->m; j++)
			glVertex3fv(sup->ponto[i][j]);
		glEnd();

		glBegin(GL_LINE_STRIP);
		for(j=0; j<sup->m; j++)
			glVertex3fv(sup->ponto[i][j]);
		glEnd();
	}

	for(i=0; i<sup->m; i++)
	{
		glBegin(GL_LINE_STRIP);
		for(j=0; j<sup->n; j++)
			glVertex3fv(sup->ponto[j][i]);
		glEnd();
	}
}

void copiarPtosControlePatch(int i0, int j0, matriz *pcPat)
{
	int i, j, jj, ii;

	// copiar n x m pontos desde (i0, j0) apartir da matriz pcObjeto
	for(i=0; i<pcPat->n; i++)
	{
		ii = i0 + i;
		for(j=0; j<pcPat->m; j++)
		{
		    jj = j0 + j;
	        pcPat->ponto[i][j][0] = pcObjeto->ponto[ii][jj][0];
			pcPat->ponto[i][j][1] = pcObjeto->ponto[ii][jj][1];
	        pcPat->ponto[i][j][2] = pcObjeto->ponto[ii][jj][2];
	        pcPat->ponto[i][j][3] = pcObjeto->ponto[ii][jj][3];
		}
	}
}

void designSurface(void)
{
	int i, j, nn, mm, idCor;

	nn = pcObjeto->n - 3;   // numero de descolamentos (patchs)

	for (i=0; i<nn; i++)
	{
		mm = pcObjeto->m - 3;
		for(j=0; j<mm; j++)
		{
		    copiarPtosControlePatch(i, j, pcPatch);  // copiar ptos de controle em matriz 4 x 4
		    ptsSuperficie(pcPatch);        // calculos pontos do PATCH com os ptos de Contrle em pcPatch 
			idCor = (i+j)%4;               // indice de paleta de COR 
		    MostrarUmPatch(idCor);
		}
	}
}


static void init(void)
{
   glClearColor(1.0, 1.0, 1.0, 0.0);
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_MAP2_VERTEX_3);
   glEnable(GL_AUTO_NORMAL);
   glMapGrid2f(20, 0.0, 1.0, 20, 0.0, 1.0);
}

void display(void)
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glPushMatrix();

   if(pcObjeto)
   {
	   MostrarPtosPoligControle(pcObjeto);  // mostrando pontos de controle
       designSurface();                   // disenhando un objeto
   }
   glutSwapBuffers();
}


void reshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      glOrtho(-10.0, 10.0, -10.0*(GLfloat)h/(GLfloat)w,
              10.0*(GLfloat)h/(GLfloat)w, -10.0, 30.0);
   else
      glOrtho(-10.0*(GLfloat)w/(GLfloat)h,
              10.0*(GLfloat)w/(GLfloat)h, -10.0, 10.0, -10.0, 30.0);
   glMatrixMode(GL_MODELVIEW);
}

void keyboard(int key, int x, int y)
{
    MatrizIdentidade();
    switch(comando){
       case Escalar:
            if (key == GLUT_KEY_DOWN) {
                matTransf[0][0] = 0.9;
                matTransf[1][1] = 0.9;
                matTransf[2][2] = 0.9;
            } else
                if (key == GLUT_KEY_UP) {
                    matTransf[0][0] = 1.1;
                    matTransf[1][1] = 1.1;
                    matTransf[2][2] = 1.1;                     
                }
            break;

        case EscalarX:
            if (key == GLUT_KEY_DOWN) {
                matTransf[0][0] = 0.9;
            } else
                if (key == GLUT_KEY_UP) {
                    matTransf[0][0] = 1.1;                   
                }
            break;

        case EscalarY:
            if (key == GLUT_KEY_DOWN) {
                matTransf[1][1] = 0.9;
            } else
                if (key == GLUT_KEY_UP) {
                    matTransf[1][1] = 1.1;                   
                }
            break;

        case EscalarZ:
            if (key == GLUT_KEY_DOWN) {
                matTransf[2][2] = 0.9;
            } else
                if (key == GLUT_KEY_UP) {
                    matTransf[2][2] = 1.1;                   
                }
            break;

        
        case RotarX:
            if(key == GLUT_KEY_LEFT)
            {  
                    matTransf[1][1] = cos(-0.1);
                    matTransf[1][2] = sin(-0.1);
                    matTransf[2][1] = -sin(-0.1);
                    matTransf[2][2] = cos(-0.1);
            } else
                if (key == GLUT_KEY_RIGHT)
                {
                    matTransf[1][1] = cos(0.1);
                    matTransf[1][2] = sin(0.1);
                    matTransf[2][1] = -sin(0.1);
                    matTransf[2][2] = cos(0.1);
                }
            break;

        case RotarY:
            if(key == GLUT_KEY_LEFT)
            {  
                    matTransf[0][0] = cos(-0.1);
                    matTransf[0][2] = -sin(-0.1);
                    matTransf[2][0] = sin(-0.1);
                    matTransf[2][2] = cos(-0.1);
            } else
                if (key == GLUT_KEY_RIGHT)
                {
                    matTransf[0][0] = cos(0.1);
                    matTransf[0][2] = -sin(0.1);
                    matTransf[2][0] = sin(0.1);
                    matTransf[2][2] = cos(0.1);
                }
            break;

        case RotarZ:
            if(key == GLUT_KEY_LEFT)
            {  
                    matTransf[0][0] = cos(-0.1);
                    matTransf[0][1] = sin(-0.1);
                    matTransf[1][0] = -sin(-0.1);
                    matTransf[1][1] = cos(-0.1);
            } else
                if (key == GLUT_KEY_RIGHT)
                {
                    matTransf[0][0] = cos(0.1);
                    matTransf[0][1] = sin(0.1);
                    matTransf[1][0] = -sin(0.1);
                    matTransf[1][1] = cos(0.1);
                }
            break;

        case TransladarX:
            if(key == GLUT_KEY_LEFT )
                    matTransf[3][0] = -0.1;
            else
                if (key == GLUT_KEY_RIGHT )
                    matTransf[3][0] = 0.1;
            break;
        case TransladarY:     
            if(key == GLUT_KEY_UP )
                    matTransf[3][1] = 0.1;
            else
                if (key == GLUT_KEY_DOWN )
                    matTransf[3][1] = -0.1;
            break;
     
   }
    MultMatriz();
    glutPostRedisplay();
}

// ------------------------------------------------------
// OBSERVACAO 4: 
//          Criar pontos de controle de n x m elementos
//          para gerar superpicies com varios PATCH
// ------------------------------------------------------
int CarregaPontos( char *arch )
{
  FILE *fobj;
  char token[40];
  float px, py, pz;
  int i, j, n, m;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0;
  }

  fgets(token, 40, fobj);
  fscanf(fobj, "%s %d %d", token, &n, &m);

  if (pcObjeto) pcObjeto = liberaMatriz(pcObjeto);

  pcObjeto =AlocaMatriz(n,m);
  
  fscanf(fobj, "%s", token);  // leitura da linha 0

  for(j=0; j<pcObjeto->n; j++)
  {
    for(i=0; i<pcObjeto->m; i++)
	 {
	     fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);

         pcObjeto->ponto[j][i][0] = px * local_scale;
         pcObjeto->ponto[j][i][1] = py * local_scale;
         pcObjeto->ponto[j][i][2] = pz * local_scale;
         pcObjeto->ponto[j][i][3] = 1.0f;
	 }
	fscanf(fobj, "%s", token);  // leitura da linha j+1;
  }

  // espaco de matriz para um patch
  if(pcPatch) pcPatch = liberaMatriz(pcPatch);
  pcPatch = AlocaMatriz(4, 4);

}

void processMenuEvents(int option)
{
    MatrizIdentidade();
    if (option == PtsControle)
        CarregaPontos( "ptosControleSuperficie4x4.txt" );  // pode ser um arquivo
    else if (option == Pontos)
        tipoView = GL_POINTS;
	else if(option == Linha)
		tipoView = GL_LINE_STRIP;
	else if (option == Solido)
		tipoView = GL_QUADS;
	else if (option == sair)
	    exit (0);
    else 
        comando = option;

    if(option==BEZIER )  
    {
		 MontaMatrizBase(option);
    }
	 else if(option==BSPLINE )  
    {
		 MontaMatrizBase(option);
    }
	 else if(option==CATMULLROM )  
    {
		 MontaMatrizBase(option);
    }
	glutPostRedisplay();
}

// -------------------------------------
// OBSERVACAO 5:
//        Adicionar opcoes de transforma��o
// ------------------------------------------
void createGLUTMenus()
{
	int menu, SUBmenuSuperficie,SUBmenuPintar,SUBmenuEscalar,SUBmenuRodar,SUBmenuTransladar;

    SUBmenuSuperficie = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Bezier", BEZIER);
	glutAddMenuEntry("BSpline", BSPLINE);
	glutAddMenuEntry("Catmullrom", CATMULLROM);


	SUBmenuPintar = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Pontos",Pontos);
	glutAddMenuEntry("Malha",Linha);
	glutAddMenuEntry("Render",Solido);

    SUBmenuRodar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("No eixo X", RotarX);
    glutAddMenuEntry("No eixo Y", RotarY);
    glutAddMenuEntry("No eixo Z", RotarZ);

	SUBmenuEscalar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Escalar", Escalar);
    glutAddMenuEntry("No eixo X", EscalarX);
    glutAddMenuEntry("No eixo Y", EscalarY);
    glutAddMenuEntry("No eixo Z", EscalarZ);

    SUBmenuTransladar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("No eixo X", TransladarX);
    glutAddMenuEntry("No eixo Y", TransladarY);

	menu = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Control Point", PtsControle);
	glutAddSubMenu("Patch", SUBmenuSuperficie);
	glutAddSubMenu("View", SUBmenuPintar);
	glutAddSubMenu("Escalar", SUBmenuEscalar);
	glutAddSubMenu("Rotacionar", SUBmenuRodar);
    glutAddSubMenu("Transladar", SUBmenuTransladar);


	glutAddMenuEntry("Quit",sair);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);


   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(700, 700);
   glutCreateWindow("Superficies ");

   init();

   glutReshapeFunc(reshape);
   glutSpecialFunc(keyboard);
   glutDisplayFunc(display);
   createGLUTMenus();

   glutMainLoop();
   return 0;
}

