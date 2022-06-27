/**
 * Uso de Bases BSplines e CatmullRom
 * Adição de operacoes: Escala, Rotacao, Translacao
 * Geracao de superficie aberto
 * Combinacao de cores e tonalidades
 * Supercicies com maior numero de patchs e formas variadas
 * Melhoria de controle de manupilacao
 * Eficiencia de realismo com triangulacao eficiente
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glut.h>

/** Tipos de desenho */
#define Line -1
#define Solid -2
#define Points -3

/** ID dos eixos das dimensões */
#define X 0
#define Y 1
#define Z 2
#define W 3

/** Funcoes de desenho */
#define Escalar 4
#define RotarX 5
#define RotarY 6
#define RotarZ 7
#define TransladaX 8
#define TransladaY 9
#define TransladaZ 10

/** Constantes de controle */
#define PtsControle 19
#define BEZIER 20
#define BSPLINE 21
#define CATMULLROM 22
#define sair 0

/** Estrutura de pontos */
typedef float f4d[4];
typedef struct st_matriz
{
  int n, m;
  f4d **ponto;
} matriz;

int comando = RotarX;
int tipoView = GL_LINE_STRIP;

float local_scale = 0.22f;
float VARIA = 0.1f;

f4d matTransf[4]; // Transposta 4 x 4
f4d MatBase[4];   // Base 4x4
f4d pView = {10.0, 10.0, -20.0, 0.0};

int idCor;

f4d vcolor[5] = {{1.0, 0.0, 0.0, 0.0},
                 {0.0, 1.0, 0.0, 0.0},
                 {0.0, 0.0, 1.0, 0.0},
                 {1.0, 1.0, 0.0, 0.0},
                 {0.0, 1.0, 1.0, 0.0}};

int windW, windH;
int gIndVert = -1;

int xinicial;
int yinicial;
int xf;
int yf;

matriz *pControle = NULL;
matriz *pcPatch = NULL;
matriz *ptsPatch = NULL;

/** Operacoes matriciais */

matriz *liberaMatriz(matriz *sup) // Controle de memoria
{
  int i;

  if (sup)
  {
    for (i = 0; i < sup->n; i++)
      free(sup->ponto[i]);

    free(sup->ponto);
    free(sup);
  }
  return NULL;
}

matriz *AlocaMatriz(int n, int m)
{
  matriz *matTemp;
  int j;

  if ((matTemp = (matriz *)malloc(sizeof(matriz))) == NULL)
  {
    printf("\n Error en alocacion de memoria para uma matriz");
    return 0;
  }

  matTemp->n = n;
  matTemp->m = m;
  matTemp->ponto = (f4d **)calloc(n, sizeof(f4d *));

  for (j = 0; j < matTemp->n; j++)
    matTemp->ponto[j] = (f4d *)calloc(m, sizeof(f4d));

  return matTemp;
}

void AlterarTransformacao(int transformType)
{
  if (transformType == BEZIER)
  {
    MatBase[0][0] = -1.0f;
    MatBase[0][1] = 3.0f;
    MatBase[0][2] = -3.0f;
    MatBase[0][3] = 1.0f;
    MatBase[1][0] = 3.0f;
    MatBase[1][1] = -6.0f;
    MatBase[1][2] = 3.0f;
    MatBase[1][3] = 0.0f;
    MatBase[2][0] = -3.0f;
    MatBase[2][1] = 3.0f;
    MatBase[2][2] = 0.0f;
    MatBase[2][3] = 0.0f;
    MatBase[3][0] = 1.0f;
    MatBase[3][1] = 0.0f;
    MatBase[3][2] = 0.0f;
    MatBase[3][3] = 0.0f;
  }

  if (transformType == BSPLINE)
  {
    MatBase[0][0] = -1.0f / 6.0;
    MatBase[0][1] = 3.0f / 6.0;
    MatBase[0][2] = -3.0f / 6.0;
    MatBase[0][3] = 1.0f / 6.0;
    MatBase[1][0] = 3.0f / 6.0;
    MatBase[1][1] = -6.0f / 6.0;
    MatBase[1][2] = 3.0f / 6.0;
    MatBase[1][3] = 0.0f;
    MatBase[2][0] = -3.0f / 6.0;
    MatBase[2][1] = 0.0f / 6.0;
    MatBase[2][2] = 3.0f / 6.0;
    MatBase[2][3] = 0.0f;
    MatBase[3][0] = 1.0f / 6.0;
    MatBase[3][1] = 4.0f / 6.0;
    MatBase[3][2] = 1.0f / 6.0;
    MatBase[3][3] = 0.0;
  }

  if (transformType == CATMULLROM)
  {
    MatBase[0][0] = -1.0f / 2.0;
    MatBase[0][1] = 3.0f / 2.0;
    MatBase[0][2] = -3.0f / 2.0;
    MatBase[0][3] = 1.0f / 2.0;
    MatBase[1][0] = 2.0f / 2.0;
    MatBase[1][1] = -5.0f / 2.0;
    MatBase[1][2] = 4.0f / 2.0;
    MatBase[1][3] = -1.0f / 2.0;
    MatBase[2][0] = -1.0f / 2.0;
    MatBase[2][1] = 0.0f / 2.0;
    MatBase[2][2] = 1.0f / 2.0;
    MatBase[2][3] = 0.0f / 2.0;
    MatBase[3][0] = 0.0f / 2.0;
    MatBase[3][1] = 2.0f / 2.0;
    MatBase[3][2] = 0.0f / 2.0;
    MatBase[3][3] = 0.0f / 2.0;
  }
}

void MatrizIdentidade()
{
  int a, b;
  for (a = 0; a < 4; a++)
  {
    for (b = 0; b < 4; b++)
    {
      if (a == b)
        matTransf[a][b] = 1;
      else
        matTransf[a][b] = 0;
    }
  }
}

void MultiplicarMatrizes()
{
  int j, k;
  f4d aux;
  for (j = 0; j < pControle->n; j++)
  {
    for (k = 0; k < pControle->m; k++)
    {
      aux[X] = pControle->ponto[j][k][X];
      aux[Y] = pControle->ponto[j][k][Y];
      aux[Z] = pControle->ponto[j][k][Z];
      aux[W] = 1.0; // pControle->ponto[j][k][W];

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

/** produção de matrizes */

void prod_VetParam_MatBase(float t, float *tt, float *vr)
{
  int i, j;

  // Vr = [t^3  t^2  t  1] [MatBase]4x4  <-- base avaliado em t.
  // ---------------------------------------------------------
  tt[0] = pow(t, 3);
  tt[1] = pow(t, 2);
  tt[2] = t;
  tt[3] = 1.0;

  for (i = 0; i < 4; i++)
  {
    vr[i] = 0.0f;
    for (j = 0; j < 4; j++)
      vr[i] += MatBase[j][i] * tt[j];
  }
}

void prod_VetMatriz(float *v, f4d **pc, f4d *vr)
{
  int i, j;

  for (i = 0; i < 4; i++)
  {
    vr[i][0] = vr[i][1] = vr[i][2] = 0.0;
    for (j = 0; j < 4; j++)
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
  float t, s;
  float tmp[4], vsm[4], vtm[4];
  f4d va[4];

  if (!pcPatch)
    return;

  n = 0;

  for (s = 0; s <= 1.01; s += VARIA)
    n += 1;

  m = n;

  if (ptsPatch)
    ptsPatch = liberaMatriz(ptsPatch);

  ptsPatch = AlocaMatriz(n, m);

  s = 0.0f;
  for (i = 0; i < ptsPatch->n; i++)
  {
    t = 0.0f;
    for (j = 0; j < ptsPatch->m; j++)
    {

      prod_VetParam_MatBase(s, tmp, vsm);
      prod_VetParam_MatBase(t, tmp, vtm);

      prod_VetMatriz(vsm, pcPatch->ponto, va);

      ptsPatch->ponto[i][j][0] = 0.0f;
      ptsPatch->ponto[i][j][1] = 0.0f;
      ptsPatch->ponto[i][j][2] = 0.0f;

      for (h = 0; h < 4; h++)
      {
        ptsPatch->ponto[i][j][0] += va[h][0] * vtm[h];
        ptsPatch->ponto[i][j][1] += va[h][1] * vtm[h];
        ptsPatch->ponto[i][j][2] += va[h][2] * vtm[h];
      }
      t += VARIA;
    }
    s += VARIA;
  }
}

void copiarPtosControlePatch(int i0, int j0, matriz *pcPat)
{
  int i, j, jj, ii;

  // copiar n x m pontos desde (i0, j0) apartir da matriz pControle
  for (i = 0; i < pcPat->n; i++)
  {
    ii = i0 + i;
    for (j = 0; j < pcPat->m; j++)
    {
      jj = j0 + j;
      pcPat->ponto[i][j][0] = pControle->ponto[ii][jj][0];
      pcPat->ponto[i][j][1] = pControle->ponto[ii][jj][1];
      pcPat->ponto[i][j][2] = pControle->ponto[ii][jj][2];
      pcPat->ponto[i][j][3] = pControle->ponto[ii][jj][3];
    }
  }
}

int CarregaPontos(char *arch)
{
  FILE *fobj;
  char token[40];
  float px, py, pz;
  int i, j, n, m;

  printf(" \n ler  %s  \n", arch);

  if ((fobj = fopen(arch, "r")) == NULL)
  {
    printf("Error en la apertura del archivo %s \n", arch);
    return 0;
  }

  fgets(token, 40, fobj);
  fscanf(fobj, "%s %d %d", token, &n, &m);

  if (pControle)
    pControle = liberaMatriz(pControle);

  pControle = AlocaMatriz(n, m);

  fscanf(fobj, "%s", token); // leitura da linha 0

  for (j = 0; j < pControle->n; j++)
  {
    for (i = 0; i < pControle->m; i++)
    {
      fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);

      pControle->ponto[j][i][0] = px * local_scale;
      pControle->ponto[j][i][1] = py * local_scale;
      pControle->ponto[j][i][2] = pz * local_scale;
      pControle->ponto[j][i][3] = 1.0f;
    }
    fscanf(fobj, "%s", token); // leitura da linha j+1;
  }

  // espaco de matriz para um patch
  if (pcPatch)
    pcPatch = liberaMatriz(pcPatch);
  pcPatch = AlocaMatriz(4, 4);
}

/** Magica do glut */

void MostrarUmPatch(int cc)
{
  int i, j;
  float t, v, s;
  f4d a, b, n, l, cx, cy, c;

  if (!ptsPatch)
    return;

  switch (tipoView)
  {
  case GL_POINTS:
    glColor3f(0.0f, 0.0f, 0.7f);
    glPointSize(1.0);
    for (i = 0; i < ptsPatch->n; i++)
    {
      glBegin(tipoView);
      for (j = 0; j < ptsPatch->m; j++)
        glVertex3fv(ptsPatch->ponto[i][j]);
      glEnd();
    }
    break;

  case GL_LINE_STRIP:
    glColor3f(0.0f, 0.0f, 0.7f);
    for (i = 0; i < ptsPatch->n; i++)
    {
      glBegin(tipoView);
      for (j = 0; j < ptsPatch->m; j++)
        glVertex3fv(ptsPatch->ponto[i][j]);
      glEnd();
    }

    for (j = 0; j < ptsPatch->n; j++)
    {
      glBegin(tipoView);
      for (i = 0; i < ptsPatch->m; i++)
        glVertex3fv(ptsPatch->ponto[i][j]);
      glEnd();
    }
    break;

  case GL_QUADS:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    for (i = 0; i < ptsPatch->n - 1; i++)
    {
      for (j = 0; j < ptsPatch->m - 1; j++)
      {

        // criando 1o triangulo
        a[X] = ptsPatch->ponto[i + 1][j][X] - ptsPatch->ponto[i][j][X];
        a[Y] = ptsPatch->ponto[i + 1][j][Y] - ptsPatch->ponto[i][j][Y];
        a[Z] = ptsPatch->ponto[i + 1][j][Z] - ptsPatch->ponto[i][j][Z];

        b[X] = ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X];
        b[Y] = ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y];
        b[Z] = ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z];

        c[X] = (ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X]) / 2 + ptsPatch->ponto[i][j][X];
        c[Y] = (ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y]) / 2 + ptsPatch->ponto[i][j][Y];
        c[Z] = (ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z]) / 2 + ptsPatch->ponto[i][j][Z];

        n[X] = a[Y] * b[Z] - a[Z] * b[Y];
        n[Y] = a[Z] * b[X] - a[X] * b[Z];
        n[Z] = a[X] * b[Y] - a[Y] * b[X];

        s = sqrt(n[X] * n[X] + n[Y] * n[Y] + n[Z] * n[Z]);

        n[X] /= s;
        n[Y] /= s;
        n[Z] /= s;

        l[X] = pView[X] - ptsPatch->ponto[i][j][X];
        l[Y] = pView[Y] - ptsPatch->ponto[i][j][Y];
        l[Z] = pView[Z] - ptsPatch->ponto[i][j][Z];

        s = n[X] * l[X] + n[Y] * l[Y] + n[Z] * l[Z];

        v = sqrt(l[X] * l[X] + l[Y] * l[Y] + l[Z] * l[Z]);
        t = s / v;

        if (t < 0.0f)
          t *= -1.00f;

        glBegin(GL_POLYGON);
        glColor3f(t * vcolor[cc][X], t * vcolor[cc][Y], t * vcolor[cc][Z]);
        glNormal3fv(n);
        glVertex3fv(ptsPatch->ponto[i][j]);
        glVertex3fv(c);
        glVertex3fv(ptsPatch->ponto[i][j + 1]);
        glEnd();

        // criando 2o triangulo
        a[X] = ptsPatch->ponto[i + 1][j][X] - ptsPatch->ponto[i][j][X];
        a[Y] = ptsPatch->ponto[i + 1][j][Y] - ptsPatch->ponto[i][j][Y];
        a[Z] = ptsPatch->ponto[i + 1][j][Z] - ptsPatch->ponto[i][j][Z];

        b[X] = ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X];
        b[Y] = ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y];
        b[Z] = ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z];

        c[X] = (ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X]) / 2 + ptsPatch->ponto[i][j][X];
        c[Y] = (ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y]) / 2 + ptsPatch->ponto[i][j][Y];
        c[Z] = (ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z]) / 2 + ptsPatch->ponto[i][j][Z];

        n[X] = a[Y] * b[Z] - a[Z] * b[Y];
        n[Y] = a[Z] * b[X] - a[X] * b[Z];
        n[Z] = a[X] * b[Y] - a[Y] * b[X];

        s = sqrt(n[X] * n[X] + n[Y] * n[Y] + n[Z] * n[Z]);

        n[X] /= s;
        n[Y] /= s;
        n[Z] /= s;

        l[X] = pView[X] - ptsPatch->ponto[i + 1][j][X];
        l[Y] = pView[Y] - ptsPatch->ponto[i + 1][j][Y];
        l[Z] = pView[Z] - ptsPatch->ponto[i + 1][j][Z];

        s = n[X] * l[X] + n[Y] * l[Y] + n[Z] * l[Z];

        v = sqrt(l[X] * l[X] + l[Y] * l[Y] + l[Z] * l[Z]);
        t = s / v;

        if (t < 0.0f)
          t *= -1.00f;

        glBegin(GL_POLYGON);
        glColor3f(t * vcolor[cc][X], t * vcolor[cc][Y], t * vcolor[cc][Z]);
        glNormal3fv(n);
        glVertex3fv(ptsPatch->ponto[i][j]);
        glVertex3fv(c);
        glVertex3fv(ptsPatch->ponto[i + 1][j]);
        glEnd();

        // criando 3o triangulo
        a[X] = ptsPatch->ponto[i + 1][j][X] - ptsPatch->ponto[i][j][X];
        a[Y] = ptsPatch->ponto[i + 1][j][Y] - ptsPatch->ponto[i][j][Y];
        a[Z] = ptsPatch->ponto[i + 1][j][Z] - ptsPatch->ponto[i][j][Z];

        b[X] = ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X];
        b[Y] = ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y];
        b[Z] = ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z];

        c[X] = (ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X]) / 2 + ptsPatch->ponto[i][j][X];
        c[Y] = (ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y]) / 2 + ptsPatch->ponto[i][j][Y];
        c[Z] = (ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z]) / 2 + ptsPatch->ponto[i][j][Z];

        n[X] = a[Y] * b[Z] - a[Z] * b[Y];
        n[Y] = a[Z] * b[X] - a[X] * b[Z];
        n[Z] = a[X] * b[Y] - a[Y] * b[X];

        s = sqrt(n[X] * n[X] + n[Y] * n[Y] + n[Z] * n[Z]);

        n[X] /= s;
        n[Y] /= s;
        n[Z] /= s;

        l[X] = pView[X] - ptsPatch->ponto[i][j + 1][X];
        l[Y] = pView[Y] - ptsPatch->ponto[i][j + 1][Y];
        l[Z] = pView[Z] - ptsPatch->ponto[i][j + 1][Z];

        s = n[X] * l[X] + n[Y] * l[Y] + n[Z] * l[Z];

        v = sqrt(l[X] * l[X] + l[Y] * l[Y] + l[Z] * l[Z]);
        t = s / v;

        if (t < 0.0f)
          t *= -1.00f;

        glBegin(GL_POLYGON);
        glColor3f(t * vcolor[cc][X], t * vcolor[cc][Y], t * vcolor[cc][Z]);
        glNormal3fv(n);
        glVertex3fv(ptsPatch->ponto[i + 1][j]);
        glVertex3fv(c);
        glVertex3fv(ptsPatch->ponto[i + 1][j + 1]);
        glEnd();

        // criando 4o triangulo
        a[X] = ptsPatch->ponto[i + 1][j][X] - ptsPatch->ponto[i][j][X];
        a[Y] = ptsPatch->ponto[i + 1][j][Y] - ptsPatch->ponto[i][j][Y];
        a[Z] = ptsPatch->ponto[i + 1][j][Z] - ptsPatch->ponto[i][j][Z];

        b[X] = ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X];
        b[Y] = ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y];
        b[Z] = ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z];

        c[X] = (ptsPatch->ponto[i + 1][j + 1][X] - ptsPatch->ponto[i][j][X]) / 2 + ptsPatch->ponto[i][j][X];
        c[Y] = (ptsPatch->ponto[i + 1][j + 1][Y] - ptsPatch->ponto[i][j][Y]) / 2 + ptsPatch->ponto[i][j][Y];
        c[Z] = (ptsPatch->ponto[i + 1][j + 1][Z] - ptsPatch->ponto[i][j][Z]) / 2 + ptsPatch->ponto[i][j][Z];

        n[X] = a[Y] * b[Z] - a[Z] * b[Y];
        n[Y] = a[Z] * b[X] - a[X] * b[Z];
        n[Z] = a[X] * b[Y] - a[Y] * b[X];

        s = sqrt(n[X] * n[X] + n[Y] * n[Y] + n[Z] * n[Z]);

        n[X] /= s;
        n[Y] /= s;
        n[Z] /= s;

        l[X] = pView[X] - ptsPatch->ponto[i + 1][j + 1][X];
        l[Y] = pView[Y] - ptsPatch->ponto[i + 1][j + 1][Y];
        l[Z] = pView[Z] - ptsPatch->ponto[i + 1][j + 1][Z];

        s = n[X] * l[X] + n[Y] * l[Y] + n[Z] * l[Z];

        v = sqrt(l[X] * l[X] + l[Y] * l[Y] + l[Z] * l[Z]);
        t = s / v;

        if (t < 0.0f)
          t *= -1.00f;

        glBegin(GL_POLYGON);
        glColor3f(t * vcolor[cc][X], t * vcolor[cc][Y], t * vcolor[cc][Z]);
        glNormal3fv(n);
        glVertex3fv(ptsPatch->ponto[i][j + 1]);
        glVertex3fv(c);
        glVertex3fv(ptsPatch->ponto[i + 1][j + 1]);
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
  for (i = 0; i < sup->n; i++)
  {
    glBegin(GL_POINTS);
    for (j = 0; j < sup->m; j++)
    {
      glVertex3fv(sup->ponto[i][j]);
    }
    glEnd();

    glBegin(GL_LINE_STRIP);
    for (j = 0; j < sup->m; j++)
      glVertex3fv(sup->ponto[i][j]);
    glEnd();
  }

  for (i = 0; i < sup->m; i++)
  {
    glBegin(GL_LINE_STRIP);
    for (j = 0; j < sup->n; j++)
      glVertex3fv(sup->ponto[j][i]);
    glEnd();
  }
}

/** Funcões do programa */

void DrawSurface(void)
{
  int i, j, nn, mm;

  nn = pControle->n - 3; // numero de descolamentos (patchs)

  for (i = 0; i < nn; i++)
  {
    mm = pControle->m - 3;
    for (j = 0; j < mm; j++)
    {
      copiarPtosControlePatch(i, j, pcPatch); // copiar ptos de controle em matriz 4 x 4
      ptsSuperficie(pcPatch);                 // calculos pontos do PATCH com os ptos de Contrle em pcPatch
      MostrarUmPatch(idCor);
    }
  }
}

void init(void)
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

  if (pControle)
  {
    MostrarPtosPoligControle(pControle); // mostrando pontos de controle
    DrawSurface();                       // disenhando un objeto
  }
  glutSwapBuffers();
}

void reshape(int w, int h)
{
  windW = w / 2;
  windH = h / 2;
  glViewport(0, 0, (GLsizei)w, (GLsizei)h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  if (w <= h)
    glOrtho(-10.0, 10.0, -10.0 * (GLfloat)h / (GLfloat)w,
            10.0 * (GLfloat)h / (GLfloat)w, -10.0, 30.0);
  else
    glOrtho(-10.0 * (GLfloat)w / (GLfloat)h,
            10.0 * (GLfloat)w / (GLfloat)h, -10.0, 10.0, -10.0, 30.0);
  glMatrixMode(GL_MODELVIEW);
}

int clipVertex(int x, int y)
{
  int i, j;
  double d;
  gIndVert = -1;
  // para cada vértice do ponto de controle
  printf("----- clipVertex -----\n");
  for (i = 0; i < pControle->n; i++)
  {
    for (j = 0; j < pControle->m; j++)
    {
      float pontoX = (double)pControle->ponto[i][j][0];
      float pontoY = (double)pControle->ponto[i][j][1];

      float dx = pontoX - x;
      dx *= local_scale;
      float dy = pontoY - y;
      dy *= local_scale;
      d = sqrt(pow((pontoX - dx), 2.0) + pow((pontoY - dy), 2.0));

      printf("%f %f %f\n", dx, dy, d);
      // distancia do ponto (x, y) a cada vértice do poligono
      // se a distancia d é bem proxima ( d < 3 pixel)

      if (d < 5.0)
      {
        gIndVert = i; // achou o indice do vértice
        break;
      }
    }
  }
  return gIndVert;
}

void scale(int key)
{
  if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN)
  {
    matTransf[0][0] = 0.99;
    matTransf[1][1] = 0.99;
    matTransf[2][2] = 0.99;
  }
  else if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP)
  {
    matTransf[0][0] = 1.01;
    matTransf[1][1] = 1.01;
    matTransf[2][2] = 1.01;
  }
}

void translateX(int key, float x)
{
  printf("translateX\n");
  printf("key: %d\n", key);
  printf("x: %f\n", x);

  if (key == GLUT_KEY_LEFT)
    matTransf[3][0] = -x;
  else if (key == GLUT_KEY_RIGHT)
    matTransf[3][0] = x;
}

void translateY(int key, float y)
{
  if (key == GLUT_KEY_UP)
    matTransf[3][1] = y;
  else if (key == GLUT_KEY_DOWN)
    matTransf[3][1] = -y;
}

void translateZ(int key)
{
  if (key == GLUT_KEY_LEFT || key == GLUT_KEY_DOWN)
  {
    matTransf[0][0] = 0.99;
    matTransf[1][1] = 0.99;
    matTransf[2][2] = 0.99;
  }
  else if (key == GLUT_KEY_RIGHT || key == GLUT_KEY_UP)
  {
    matTransf[0][0] = 1.01;
    matTransf[1][1] = 1.01;
    matTransf[2][2] = 1.01;
  }
}

void rotateX(int key)
{
  if (key == GLUT_KEY_LEFT)
  {
    matTransf[1][1] = cos(-0.01);
    matTransf[1][2] = sin(-0.01);
    matTransf[2][1] = -sin(-0.01);
    matTransf[2][2] = cos(-0.01);
  }
  else if (key == GLUT_KEY_RIGHT)
  {
    matTransf[1][1] = cos(0.01);
    matTransf[1][2] = sin(0.01);
    matTransf[2][1] = -sin(0.01);
    matTransf[2][2] = cos(0.01);
  }
}

void rotateY(int key)
{
  if (key == GLUT_KEY_UP)
  {
    matTransf[0][0] = cos(-0.01);
    matTransf[0][2] = sin(-0.01);
    matTransf[2][0] = -sin(-0.01);
    matTransf[2][2] = cos(-0.01);
  }
  else if (key == GLUT_KEY_DOWN)
  {
    matTransf[0][0] = cos(0.01);
    matTransf[0][2] = sin(0.01);
    matTransf[2][0] = -sin(0.01);
    matTransf[2][2] = cos(0.01);
  }
}

void rotateZ(int key)
{
  if (key == GLUT_KEY_LEFT)
  {
    matTransf[0][0] = cos(-0.01);
    matTransf[0][1] = sin(-0.01);
    matTransf[1][0] = -sin(-0.01);
    matTransf[1][1] = cos(-0.01);
  }
  else if (key == GLUT_KEY_RIGHT)
  {
    matTransf[0][0] = cos(0.01);
    matTransf[0][1] = sin(0.01);
    matTransf[1][0] = -sin(0.01);
    matTransf[1][1] = cos(0.01);
  }
}

void motion(int x, int y)
{
  int i;
  float dx, dy;
  x = x - windW;
  y = windH - y;

  dx = x - xinicial;
  dy = y - yinicial;

  int ladoX = 0, ladoY = 0;

  switch (comando)
  {
  case Escalar:
    if (dy > 0)
    {
      ladoY = GLUT_KEY_UP;
    }
    else
    {
      ladoY = GLUT_KEY_DOWN;
    }

    scale(ladoY);
    break;
  case RotarX:
    if (dx > 0)
    {
      ladoX = GLUT_KEY_RIGHT;
    }
    else
    {
      ladoX = GLUT_KEY_LEFT;
    }

    rotateX(ladoX);
    break;
  case RotarY:
    if (dy > 0)
    {
      ladoY = GLUT_KEY_UP;
    }
    else
    {
      ladoY = GLUT_KEY_DOWN;
    }

    rotateY(ladoY);
    break;
  case RotarZ:
    if (dx > 0)
    {
      ladoX = GLUT_KEY_RIGHT;
    }
    else
    {
      ladoX = GLUT_KEY_LEFT;
    }

    rotateZ(ladoX);
    break;
  case TransladaX:
    if (dx > 0)
    {
      ladoX = GLUT_KEY_RIGHT;
    }
    else
    {
      ladoX = GLUT_KEY_LEFT;
    }

    translateX(ladoX, 0.1);
    break;
  case TransladaY:
    if (dy > 0)
    {
      ladoY = GLUT_KEY_UP;
    }
    else
    {
      ladoY = GLUT_KEY_DOWN;
    }

    translateY(ladoY, 0.1);
    break;
  case TransladaZ:
    if (dy > 0)
    {
      ladoY = GLUT_KEY_UP;
    }
    else
    {
      ladoY = GLUT_KEY_DOWN;
    }

    translateZ(ladoY);
    break;
  }

  xinicial = dx;
  yinicial = dy;

  MultiplicarMatrizes();
  glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
  if (state == GLUT_DOWN) //  botão SOLTO
  {
    if (button == GLUT_LEFT_BUTTON)
    {
      xinicial = x - windW;
      yinicial = windH - y;
    }
  }

  if (state == GLUT_UP)
  {
    if (button == GLUT_LEFT_BUTTON)
    {
      xf = x - windW;
      yf = windH - y;
    }
  }

  glutPostRedisplay();
}

void keyboard(int key, int x, int y)
{
  MatrizIdentidade(); // identidade em matTransf : matriz de transforma
  switch (comando)
  {
  case Escalar:
    scale(key);
    break;
  case RotarX:
    rotateX(key);
    break;
  case RotarY:
    rotateY(key);
    break;
  case RotarZ:
    rotateZ(key);
    break;
  case TransladaX:
    translateX(key, 0.1);
    break;
  case TransladaY:
    translateY(key, 0.1);
    break;
  case TransladaZ:
    translateZ(key);
    break;
  }
  MultiplicarMatrizes();
  glutPostRedisplay();
}

void processMenuEvents(int option)
{

  MatrizIdentidade();
  if (option == PtsControle)
    CarregaPontos("/home/augusto/dev/home/old/riveira-compgrafica/trab23/44pto.txt"); // ptosControleArbitra4x4   ptosControleSuperficieTrab ptosControleSuperficie4x4
  else if (option == Points)
    tipoView = GL_POINTS;
  else if (option == Line)
    tipoView = GL_LINE_STRIP;
  else if (option == Solid)
    tipoView = GL_QUADS;
  else if (option == sair)
    exit(0);
  else
    comando = option;

  if (option == BEZIER || option == BSPLINE || option == CATMULLROM) // OBSERVACAO: considerar cado de CATMULLROM
  {
    AlterarTransformacao(option);
  }
  glutPostRedisplay();
}

void processColorEvents(int option)
{
  switch (option)
  {
  case 0:
    idCor = 0;
    break;
  case 1:
    idCor = 1;
    break;
  case 2:
    idCor = 2;
    break;
  case 3:
    idCor = 3;
    break;
  case 4:
    idCor = 4;
    break;
  default:
    idCor = 0;
    break;
  }
}

void createGLUTMenus()
{
  int menu, submenu, SUBmenuTransladar, SUBmenuGirar, SUBmenuSuperficie, SUBmenuPintar, SUBmenuCores;

  SUBmenuSuperficie = glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Bezier", BEZIER);
  glutAddMenuEntry("B-Spline", BSPLINE);
  glutAddMenuEntry("CatmullRom", CATMULLROM);

  SUBmenuGirar = glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("EixoX", RotarX);
  glutAddMenuEntry("EixoY", RotarY);
  glutAddMenuEntry("EixoZ", RotarZ);

  SUBmenuTransladar = glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("EixoX", TransladaX);
  glutAddMenuEntry("EixoY", TransladaY);
  glutAddMenuEntry("EixoZ", TransladaZ);

  SUBmenuPintar = glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Pontos", Points);
  glutAddMenuEntry("Malha", Line);
  glutAddMenuEntry("Preenchido", Solid);

  SUBmenuCores = glutCreateMenu(processColorEvents);
  glutAddMenuEntry("Vermelho", 0);
  glutAddMenuEntry("Verde", 1);
  glutAddMenuEntry("Azul", 2);
  glutAddMenuEntry("Amarelo", 3);
  glutAddMenuEntry("Ciano", 4);

  menu = glutCreateMenu(processMenuEvents);
  glutAddMenuEntry("Control Point ...", PtsControle);
  glutAddSubMenu("Surfaces", SUBmenuSuperficie);
  glutAddSubMenu("Objet View", SUBmenuPintar);
  glutAddSubMenu("Cores", SUBmenuCores);
  glutAddMenuEntry("Scale", Escalar);
  glutAddSubMenu("Rotate", SUBmenuGirar);
  glutAddSubMenu("Translate", SUBmenuTransladar);
  glutAddMenuEntry("Quit", sair);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(700, 700);
  glutCreateWindow("Superficies ");

  init();

  glutReshapeFunc(reshape);
  glutSpecialFunc(keyboard);
  glutMotionFunc(motion);
  glutMouseFunc(mouse);
  glutDisplayFunc(display);
  createGLUTMenus();

  glutMainLoop();
  return 0;
}
