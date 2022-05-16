// ------------------------------------------------------------------------------------
// Trabalho: 
//        - Geracao de superficie aberto 
//        - Adição de operacoes: Escala, Rotacao, Translacao ok 
//        - Uso de Bases BSplines e CatmullRom ok
//        - Eficiencia de realismo com triangulacao eficiente
//        - Combinacao de cores e tonalidades
//        - Supercicies com maior numero de patchs e formas variadas ok
//        - Melhoria de controle de manupilacao (mouse? ao inves de teclado?)
// ------------------------------------------------------------------------------------
// Atender as 7 OBSERVACOES ....
// ------------------------------------------------------------------------------------
// Data de entrega e apresentacao: terçca feira 4 de fev
// Trabalho individual (similares Uma nota dividda em numeo de similares)
// -------------------------------------------------------------------------------------                 

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>

#define Linha -1
#define Solido -2 
#define Pontos -3

#define X 0
#define Y 1
#define Z 2
#define W 3

#define Escalar 4 // Redimensionar
#define EscalarX 5
#define EscalarY 6
#define EscalarZ 7


#define RotarX 8
#define RotarY 9
#define RotarZ 10

#define TransladaX 11
#define TransladaY 12
#define TransladaZ 13

#define PtsControle 19
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

float VARIA = 0.25f;


f4d matTransf[4];

f4d MatBase[4];   // matriz de base 

f4d pView = {10.0, 10.0, -20.0, 0.0};

    // ---------------------------------------------
    // OBSERVACAO 0: cores
    //       definir mais cores 
    // ---------------------------------------------
f4d vcolor[8] = {{1.0, 0.0, 0.0, 0.0},
                 {0.0, 1.0, 0.0, 0.0},
                 {0.0, 0.0, 1.0, 0.0}, 
                 {1.0, 0.0, 1.0, 0.0},   //rosa
                 {0.5, 0.0, 0.0, 0.0},   //cinza
                 {1.0, 0.65, 0.0, 0.0},  //laranja
                 {0.6, 0.2, 0.8, 0.0},   //roxo
                 {1.0, 1.0, 0.0, 0.0}};


matriz *pControle = NULL;  // matriz de pontos de controle  LIDOS

matriz *pcPatch = NULL;    // matriz de pontos para um patch 

matriz *ptsPatch = NULL;   // matriiz de PONTOS na superficie PATCH calclulados

void DisenaSuperficie(void);

// ----------------------------------------------------------------------------
// OBSERVACAO 1:
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
    for(j=0; j< pControle->n; j++)
    {
        for(k = 0; k< pControle->m; k++)
        {
         aux[X] = pControle->ponto[j][k][X]; 
         aux[Y] = pControle->ponto[j][k][Y]; 
         aux[Z] = pControle->ponto[j][k][Z];
         aux[W] = 1.0; //pControle->ponto[j][k][W];

         //  Pj = MatTransf4x4 . Pj  <--- transformada homogenea 

         pControle->ponto[j][k][X] = matTransf[X][X] * aux[X] + 
                                     matTransf[Y][X] * aux[Y] + 
                                     matTransf[Z][X] * aux[Z] +
                                     matTransf[W][X] * aux[W];

         pControle->ponto[j][k][Y] = matTransf[X][Y] * aux[X] + 
                                     matTransf[Y][Y] * aux[Y] + 
                                     matTransf[Z][Y] * aux[Z] +
                                     matTransf[W][Y] * aux[W];

         pControle->ponto[j][k][Z] = matTransf[X][Z] * aux[X] + 
                                     matTransf[Y][Z] * aux[Y] + 
                                     matTransf[Z][Z] * aux[Z] +
                                     matTransf[W][Z] * aux[W];

         pControle->ponto[j][k][W] = matTransf[X][W] * aux[X] + 
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

            for(h=0; h<4; h++)                      // p = S G P G^t T = va vtm
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
    float t,v,s, ang1, ang2;
    f4d a,b,n,l;
    f4d n1,n2,n3,n4;

    if(!ptsPatch)  return;

    switch(tipoView)
    {
        case GL_POINTS:
          glColor3f(0.0f, 0.8f, 0.0f);
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
          glColor3f(0.0f, 0.8f, 0.0f);
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
    // OBSERVACAO 2: modificar
    //  Ver a melhor forma de criar triangulos 1ro e 2do
    // -----------------------------------------------------

                //1ºTRIANGULO

                a[X] = ptsPatch->ponto[i][j][X] - ptsPatch->ponto[i+1][j][X];
                a[Y] = ptsPatch->ponto[i][j][Y] - ptsPatch->ponto[i+1][j][Y];
                a[Z] = ptsPatch->ponto[i][j][Z] - ptsPatch->ponto[i+1][j][Z];

                b[X] = ptsPatch->ponto[i+1][j+1][X] - ptsPatch->ponto[i+1][j][X];
                b[Y] = ptsPatch->ponto[i+1][j+1][Y] - ptsPatch->ponto[i+1][j][Y];
                b[Z] = ptsPatch->ponto[i+1][j+1][Z] - ptsPatch->ponto[i+1][j][Z];

                n1[X] = a[Y]*b[Z] - a[Z]*b[Y];
                n1[Y] = a[Z]*b[X] - a[X]*b[Z];
                n1[Z] = a[X]*b[Y] - a[Y]*b[X];

                

                //2ºTRIANGULO

                a[X] = ptsPatch->ponto[i][j][X] - ptsPatch->ponto[i][j+1][X];
                a[Y] = ptsPatch->ponto[i][j][Y] - ptsPatch->ponto[i][j+1][Y];
                a[Z] = ptsPatch->ponto[i][j][Z] - ptsPatch->ponto[i][j+1][Z];

                b[X] = ptsPatch->ponto[i+1][j+1][X] - ptsPatch->ponto[i][j+1][X];
                b[Y] = ptsPatch->ponto[i+1][j+1][Y] - ptsPatch->ponto[i][j+1][Y];
                b[Z] = ptsPatch->ponto[i+1][j+1][Z] - ptsPatch->ponto[i][j+1][Z];

                n2[X] = a[Y]*b[Z] - a[Z]*b[Y];
                n2[Y] = a[Z]*b[X] - a[X]*b[Z];
                n2[Z] = a[X]*b[Y] - a[Y]*b[X];            

    
                  // criando 1ro triangulo do quadrilatero

                a[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i][j][X];
                a[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i][j][Y];
                a[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i][j][Z];

                b[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i][j][X];
                b[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i][j][Y];
                b[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i][j][Z];

                n3[X] = a[Y]*b[Z] - a[Z]*b[Y];
                n3[Y] = a[Z]*b[X] - a[X]*b[Z];
                n3[Z] = a[X]*b[Y] - a[Y]*b[X];


                // criando 2do triangulo


                a[X] = ptsPatch->ponto[i][j+1][X] - ptsPatch->ponto[i+1][j+1][X];
                a[Y] = ptsPatch->ponto[i][j+1][Y] - ptsPatch->ponto[i+1][j+1][Y];
                a[Z] = ptsPatch->ponto[i][j+1][Z] - ptsPatch->ponto[i+1][j+1][Z];

                b[X] = ptsPatch->ponto[i+1][j][X] - ptsPatch->ponto[i+1][j+1][X];
                b[Y] = ptsPatch->ponto[i+1][j][Y] - ptsPatch->ponto[i+1][j+1][Y];
                b[Z] = ptsPatch->ponto[i+1][j][Z] - ptsPatch->ponto[i+1][j+1][Z];

                n4[X] = a[Y]*b[Z] - a[Z]*b[Y];
                n4[Y] = a[Z]*b[X] - a[X]*b[Z];
                n4[Z] = a[X]*b[Y] - a[Y]*b[X];

               //angulo entre normais

                ang1 = (n1[X]*n2[X]+n1[Y]*n2[Y]+n1[Z]*n2[Z]);
                ang2 = (n3[X]*n4[X]+n3[Y]*n4[Y]+n3[Z]*n4[Z]);
                

                if(ang1>ang2){

                    s = sqrt(n1[X]*n1[X]+n1[Y]*n1[Y]+n1[Z]*n1[Z]);

                    n1[X] /=s; n1[Y] /=s; n1[Z] /=s;

                    l[X] = pView[X] - ptsPatch->ponto[i+1][j][X];
                    l[Y] = pView[Y] - ptsPatch->ponto[i+1][j][Y];
                    l[Z] = pView[Z] - ptsPatch->ponto[i+1][j][Z];

                    s = n1[X]*l[X]+n1[Y]*l[Y]+n1[Z]*l[Z];

                    v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
                    t = s / v;

                    if(t<0.0f)
                        t *= -1.00f;

                        glBegin(GL_POLYGON);
                            glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
                            glNormal3fv(n1);
                            glVertex3fv(ptsPatch->ponto[i][j]);
                            glVertex3fv(ptsPatch->ponto[i][j+1]);
                            glVertex3fv(ptsPatch->ponto[i+1][j]);
                        glEnd();

                    s = sqrt(n2[X]*n2[X]+n2[Y]*n2[Y]+n2[Z]*n2[Z]);

                    n2[X] /=s; n2[Y] /=s; n2[Z] /=s;

                    l[X] = pView[X] - ptsPatch->ponto[i][j+1][X];
                    l[Y] = pView[Y] - ptsPatch->ponto[i][j+1][Y];
                    l[Z] = pView[Z] - ptsPatch->ponto[i][j+1][Z];

                    s = n2[X]*l[X]+n2[Y]*l[Y]+n2[Z]*l[Z];

                    v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
                    t = s / v;  

                    if(t<0.0f)
                        t *= -1.00f;

                        glBegin(GL_POLYGON);
                            glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
                            glNormal3fv(n2);
                            glVertex3fv(ptsPatch->ponto[i][j+1]);
                            glVertex3fv(ptsPatch->ponto[i+1][j+1]);
                            glVertex3fv(ptsPatch->ponto[i+1][j]);
                        glEnd();

                }

                else{

                    s = sqrt(n3[X]*n3[X]+n3[Y]*n3[Y]+n3[Z]*n3[Z]);

                    n3[X] /=s; n3[Y] /=s; n3[Z] /=s;

                    l[X] = pView[X] - ptsPatch->ponto[i][j][X];
                    l[Y] = pView[Y] - ptsPatch->ponto[i][j][Y];
                    l[Z] = pView[Z] - ptsPatch->ponto[i][j][Z];

                    s = n3[X]*l[X]+n3[Y]*l[Y]+n3[Z]*l[Z];

                    v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
                    t = s / v;
                   
                    if(t<0.0f)
                        t *= -1.00f;

                        glBegin(GL_POLYGON);
                            glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
                            glNormal3fv(n3);
                            glVertex3fv(ptsPatch->ponto[i][j]);
                            glVertex3fv(ptsPatch->ponto[i+1][j]);
                            glVertex3fv(ptsPatch->ponto[i+1][j+1]);
                        glEnd();

                    s = sqrt(n4[X]*n4[X]+n4[Y]*n4[Y]+n4[Z]*n4[Z]);

                    n4[X] /=s; n4[Y] /=s; n4[Z] /=s;

                    l[X] = pView[X] - ptsPatch->ponto[i+1][j+1][X];
                    l[Y] = pView[Y] - ptsPatch->ponto[i+1][j+1][Y];
                    l[Z] = pView[Z] - ptsPatch->ponto[i+1][j+1][Z];

                    s = n4[X]*l[X]+n4[Y]*l[Y]+n4[Z]*l[Z];

                    v = sqrt(l[X]*l[X]+l[Y]*l[Y]+l[Z]*l[Z]);
                    t = s / v;    

                    if(t<0.0f)
                        t *= -1.00f;

                        glBegin(GL_POLYGON);
                            glColor3f(t*vcolor[cc][X],t*vcolor[cc][Y],t*vcolor[cc][Z]);
                            glNormal3fv(n4);
                            glVertex3fv(ptsPatch->ponto[i][j]);
                            glVertex3fv(ptsPatch->ponto[i][j+1]);
                            glVertex3fv(ptsPatch->ponto[i+1][j+1]);
                        glEnd();
                    }
              }
          }
          break;
    }

}

void MostrarPtosPoligControle(matriz *sup)
{
    int i, j;

    glColor3f(1.0f, 0.0f, 1.0f); //0.0f, 0.8f, 0.0f
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

    // copiar n x m pontos desde (i0, j0) apartir da matriz pControle
    for(i=0; i<pcPat->n; i++)
    {
        ii = i0 + i;
        for(j=0; j<pcPat->m; j++)
        {
            jj = j0 + j;
            pcPat->ponto[i][j][0] = pControle->ponto[ii][jj][0];
            pcPat->ponto[i][j][1] = pControle->ponto[ii][jj][1];
            pcPat->ponto[i][j][2] = pControle->ponto[ii][jj][2];
            pcPat->ponto[i][j][3] = pControle->ponto[ii][jj][3];
        }
    }
}

void DisenaSuperficie(void)
{
    int i, j, nn, mm, idCor;

    nn = pControle->n - 3;   // numero de descolamentos (patchs)

    for (i=0; i<nn; i++)
    {
        mm = pControle->m - 3;
        for(j=0; j<mm; j++)
        {
            copiarPtosControlePatch(i, j, pcPatch);  // copiar ptos de controle em matriz 4 x 4
            ptsSuperficie(pcPatch);        // calculos pontos do PATCH com os ptos de Contrle em pcPatch 
            idCor = (i+j)%8;               // indice de paleta de COR 
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

   if(pControle)
   {
       MostrarPtosPoligControle(pControle);  // mostrando pontos de controle
       DisenaSuperficie();                   // disenhando un objeto
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
    MatrizIdentidade();  // identidade em matTransf : matriz de transforma
    switch(comando){
       case Escalar:
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN) {
                matTransf[0][0] = 0.95;
                matTransf[1][1] = 0.95;
                matTransf[2][2] = 0.95;
            } else
                if ( key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
                    matTransf[0][0] = 1.05;
                    matTransf[1][1] = 1.05;
                    matTransf[2][2] = 1.05;                     
                }
            break;

        case EscalarX:
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN) {
                matTransf[0][0] = 0.95;
            } else
                if ( key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
                    matTransf[0][0] = 1.05;                   
                }
            break;

        case EscalarY:
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN) {
                matTransf[1][1] = 0.95;
            } else
                if ( key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
                    matTransf[1][1] = 1.05;                   
                }
            break;

        case EscalarZ:
            if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN) {
                matTransf[2][2] = 0.95;
            } else
                if ( key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP) {
                    matTransf[2][2] = 1.05;                   
                }
            break;

        case RotarX:
            if(key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN )
            {  
                    matTransf[1][1] = cos(-0.2);
                    matTransf[1][2] = sin(-0.2);
                    matTransf[2][1] = -sin(-0.2);
                    matTransf[2][2] = cos(-0.2);
            } else
                if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP )
                {
                    matTransf[1][1] = cos(0.2);
                    matTransf[1][2] = sin(0.2);
                    matTransf[2][1] = -sin(0.2);
                    matTransf[2][2] = cos(0.2);
                }
            break;

        case RotarY:
            if(key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN )
            {  
                    matTransf[0][0] = cos(-0.2);
                    matTransf[0][2] = -sin(-0.2);
                    matTransf[2][0] = sin(-0.2);
                    matTransf[2][2] = cos(-0.2);
            } else
                if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP )
                {
                    matTransf[0][0] = cos(0.2);
                    matTransf[0][2] = -sin(0.2);
                    matTransf[2][0] = sin(0.2);
                    matTransf[2][2] = cos(0.2);
                }
            break;

        case RotarZ:
            if(key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN )
            {  
                    matTransf[0][0] = cos(-0.2);
                    matTransf[0][1] = sin(-0.2);
                    matTransf[1][0] = -sin(-0.2);
                    matTransf[1][1] = cos(-0.2);
            } else
                if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP )
                {
                    matTransf[0][0] = cos(0.2);
                    matTransf[0][1] = sin(0.2);
                    matTransf[1][0] = -sin(0.2);
                    matTransf[1][1] = cos(0.2);
                }
            break;

    //-------------------------------------------------------
    // OBSERVACAO 3:  
    // Considerar Rotacao no eixo Y  e  Z
    // -----------------------------------------------------

        case TransladaX:
            if(key == GLUT_KEY_LEFT )
                    matTransf[3][0] = -0.50;
            else
                if (key == GLUT_KEY_RIGHT )
                    matTransf[3][0] = 0.50;
            break;
        case TransladaY:     
            if(key == GLUT_KEY_UP )
                    matTransf[3][1] = 0.50;
            else
                if (key == GLUT_KEY_DOWN )
                    matTransf[3][1] = -0.50;
            break;
        case TransladaZ:     
            if(key == GLUT_KEY_UP )
                    matTransf[3][2] = 0.50;
            else
                if (key == GLUT_KEY_DOWN )
                    matTransf[3][2] = -0.50;
            break;
    //-------------------------------------------------------
    // OBSERVACAO 4:  
    // Considerar Tranlacao no eixo X  y  Z
    // -----------------------------------------------------
   }
    MultMatriz();
    glutPostRedisplay();
}

int CarregaPontos( char *arch)
{
  FILE *fobj;
  char token[100];
  float px, py, pz;
  int i, j, n, m;

  printf(" \n ler  %s  \n", arch);

  if((fobj=fopen(arch,"rt"))==NULL)
  {
     printf("Error en la apertura del archivo %s \n", arch);
     return 0;
  }

  fgets(token, 100, fobj);
  fscanf(fobj, "%s %d %d", token, &n, &m);

  if (pControle) pControle = liberaMatriz(pControle);

  pControle =AlocaMatriz(n,m);
  
  fscanf(fobj, "%s", token);  // leitura da linha 0

  for(j=0; j<pControle->n; j++)
  {
    for(i=0; i<pControle->m; i++)
     {
         fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);

         pControle->ponto[j][i][0] = px * local_scale;
         pControle->ponto[j][i][1] = py * local_scale;
         pControle->ponto[j][i][2] = pz * local_scale;
         pControle->ponto[j][i][3] = 1.0f;
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
        CarregaPontos("ptosControleSuperficieTrab.txt"); 
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

    if(option==BEZIER || option==BSPLINE || option==CATMULLROM)  // OBSERVACAO: considerar cado de CATMULLROM
    {
         MontaMatrizBase(option);
    }
    glutPostRedisplay();
}

void createGLUTMenus()
{
    int menu, submenu, SUBmenuTransladar, SUBmenuGirar,SUBmenuSuperficie,SUBmenuPintar,SUBmenuEscalar;

    SUBmenuSuperficie = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Bezier", BEZIER);
    glutAddMenuEntry("B-Spline", BSPLINE);
    glutAddMenuEntry("CatmullRom", CATMULLROM);
    // -----------------------------------------------------
    // OBSERVACAO 5:  
    // colocar opcao para CatmullRom
    // -----------------------------------------------------

    
    SUBmenuGirar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("EixoX", RotarX);
    glutAddMenuEntry("EixoY", RotarY);
    glutAddMenuEntry("EixoZ", RotarZ);

    // ----------------------------------------------------
    // OBSERVACAO 6:
    // Considerar opcao de Rotacao em Y e Z
    // ------------------------------------------------------

    SUBmenuTransladar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("EixoX", TransladaX);
    glutAddMenuEntry("EixoY", TransladaY);
    glutAddMenuEntry("EixoZ", TransladaZ);
    // ----------------------------------------------------
    // OBSERVACAO 7:
    // Considerar opcao de Translacao em Y e Z
    // ------------------------------------------------------

    SUBmenuPintar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Pontos",Pontos);
    glutAddMenuEntry("Malha",Linha);
    glutAddMenuEntry("Preenchido",Solido);

    SUBmenuEscalar = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Redimensionar", Escalar);
    glutAddMenuEntry("EixoX", EscalarX);
    glutAddMenuEntry("EixoY", EscalarY);
    glutAddMenuEntry("EixoZ", EscalarZ);


    menu = glutCreateMenu(processMenuEvents);
    glutAddMenuEntry("Pontos de Controle", PtsControle);
    glutAddSubMenu("Superficies", SUBmenuSuperficie);
    glutAddSubMenu("Object View", SUBmenuPintar);
    glutAddSubMenu("Redimensionar", SUBmenuEscalar);
    glutAddSubMenu("Rotacionar", SUBmenuGirar);
    glutAddSubMenu("Transladar", SUBmenuTransladar);
    glutAddMenuEntry("Sair",sair);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char** argv)
{
   glutInit(&argc, argv);

   glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
   glutInitWindowSize(700, 700);
   glutCreateWindow("Superficies.C");

   init();

   glutReshapeFunc(reshape);
   glutSpecialFunc(keyboard);
   glutDisplayFunc(display);
   createGLUTMenus();

   glutMainLoop();
   return 0;
}
