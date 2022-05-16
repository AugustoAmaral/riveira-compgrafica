// ------------------------------ trabalho ------------------------------------
// Realismo e animacao por particulas e molas
// Realismo: cada objeto em cores diferentes... tonalidades, brilho, sombras, etc.
// Anima��o: completar a animacao de particulas que esta em passoEDOresolve
//           caso 1 ate caso 4
// estar com outros objetos
// --------------------------------------------------------------------------

#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

#define	SIZE	10.0

typedef float f4d[4];

typedef struct stVetorF4d
{
	int n;			// numero de vertices
	f4d *vPos;	// coordenadas de vertice
} vetPontos;

typedef struct stStatePart
{
	float m;
	f4d vel;
	f4d forc;
	f4d oldPos;
	f4d oldVel;
} statePart;

typedef struct stVetorPart
{
	int n;
	statePart *mvf;
} vetPart;

typedef struct stVetVertex
{
	int n;			// numero de elementos
	int *vInd;	// elemento - indice del vertice
	f4d vNormal;    // vetor normal da face
} vetVetexFace;

typedef struct stVetFaces
{
	int n;						// numero de faces
	vetVetexFace *face;	// faces por indice vertices
} vetFace;

typedef struct stVetMola
{
	int a, b;		// indice del vertice (extremos)
	float r;    	// comprimento em repouso
	float Ks, Kd;	// constate de spring e damp ks
} vetMola;

typedef struct stVetArestas
{
	int n;						// numero de faces
	vetMola *mola;	// faces por indice vertices
} vetArest;

typedef struct stUmObjeto
{
	vetPontos *Vertices;		// vector de vertices (posicao
	vetFace *Faces;		// vector de faces
	vetPart *Parts;			// vetor de particulas
	vetArest *Molas;		// vetor de aresta - mola
} umObjeto;

// estrutura temporaria de lista
typedef struct stlist{
	int a, b;
	struct stlist *next;
} stLst;


// ===========================================

umObjeto *objeto = NULL;		// un objeto dinamico
umObjeto *objFixo = NULL;		// um objeto fixo

// ===========================================
f4d gvPosBase = {0.0, -50.0, 0.0};    	// posicao para o objto fixo
float local_scale = 1.0f;
int RendMode; /* May take values 1 or 2: for wireframe and polygon modes. */
double alpha, beta;  /* Angles of rotation about vertical and horizontal axes. */

float gConsRest = 0.7;	// [0.0, 1.0]constante de restitucao da face de colisao
	
float gKd = 0.7;	// constante de damp
float gKs = 0.1;    // constante de spring
float gMassa = 5.0; // masa da particula;
f4d	gGrav = {0.0, -9.8, 0.0};	// acelracao de gravidade (0, -9.8, 0) dado em inicial
float gDt = 0.05;	// passo de tempo
int gAnima = 0;		// indicados para animar 0 nao, 1 (sim)

// Defining a point light source parameters
GLfloat light_position1[] = {   0.0,   10.0*SIZE, 100.0*SIZE, 1.0 };
GLfloat light_ambient[]   = {   0.1,   0.1,   0.1, 1.0 };
GLfloat light_diffuse[]   = {   1.0,   1.0,   1.0, 1.0 };
GLfloat light_specular[]  = {   1.0,   1.0,   1.0, 1.0 };

//Adicionamos aqui
GLfloat mat_ambiente[] = {0.3, 0.0, 0.0, 1.0};
GLfloat mat_difuso[] = {0.5, 0.0, 0.0, 1.0};
GLfloat mat_especular[] = {1.0, 1.0, 1.0, 1.0};
GLfloat mat_emissao[] = {0, 0, 0, 1};
GLfloat mat_brilho[] = {50.0};
//============================================================

float calNorma(f4d vv) {
	float s;
	s = sqrt(vv[0]*vv[0]+vv[1]*vv[1]+vv[2]*vv[2]);
	return s;
}

float calDistancia(f4d pa, f4d pb) {
	f4d dd;
	dd[0] = pa[0] - pb[0];
	dd[1] = pa[1] - pb[1];
	dd[2] = pa[2] - pb[2];	
	return(calNorma(dd));
}

float calProdutoEscalar(f4d vA, f4d vB) {
	float s;
	s = vA[0]*vB[0] + vA[1]*vB[1] + vA[2]*vB[2];
	return s;
}

// Face Normal Computation

void calculaNormalFace(vetVetexFace *fac, vetPontos *vert)
{
	f4d a, b, vNorm;
	float s;
	int nn, io, ia, ib;
	
	nn = fac->n - 1;
	io = fac->vInd[0];
	ia = fac->vInd[1];
	ib = fac->vInd[nn];

//	io = fac->vInd[1];
//	ia = fac->vInd[0];
//	ib = fac->vInd[2];

	a[0] = vert->vPos[ia][0] - vert->vPos[io][0];
	a[1] = vert->vPos[ia][1] - vert->vPos[io][1];
	a[2] = vert->vPos[ia][2] - vert->vPos[io][2];

	b[0] = vert->vPos[ib][0] - vert->vPos[io][0];
	b[1] = vert->vPos[ib][1] - vert->vPos[io][1];
	b[2] = vert->vPos[ib][2] - vert->vPos[io][2];
	
	vNorm[0] = a[1]*b[2] - a[2]*b[1];
	vNorm[1] = a[2]*b[0] - a[0]*b[2];
	vNorm[2] = a[0]*b[1] - a[1]*b[0];

	s = calNorma(vNorm);

	fac->vNormal[0] = vNorm[0] / s;
	fac->vNormal[1] = vNorm[1] / s;
	fac->vNormal[2] = vNorm[2] / s;

}

// ===========================================================

// Design a object 

void DesenhaObjeto(umObjeto *obj)
{
	int i, j, h;
	f4d vn;

	if(!obj)
		return;

	//glPolygonMode(GL_FRONT_AND_BACK, RendMode);
	glColor3f(0.76f, 0.18f, 0.61f);
	for(i=0; i<obj->Faces->n; i++)
	{
		glBegin(GL_POLYGON);
		if(RendMode>1) {
			calculaNormalFace(&(obj->Faces->face[i]), obj->Vertices); // computa normal da face
			glNormal3fv(obj->Faces->face[i].vNormal);
			vn[0] = obj->Faces->face[i].vNormal[0];
			vn[1] = obj->Faces->face[i].vNormal[1];
			vn[2] = obj->Faces->face[i].vNormal[2];
		}
		for(j=0; j<obj->Faces->face[i].n; j++)
		{
			h = obj->Faces->face[i].vInd[j];
			glVertex3fv(obj->Vertices->vPos[h]);
		}
		glEnd();	
	} 
} 
// ===========================================================

umObjeto *criaObjeto(void) {
	umObjeto *obj;
	obj = (umObjeto*) malloc(sizeof(umObjeto));
	obj->Faces = NULL;
	obj->Molas = NULL;
	obj->Parts = NULL;
	obj->Vertices = NULL;
	return obj;
}

umObjeto* liberaObjeto(umObjeto* obj)
{
	int i, j;

	if(obj)
	{
		if(obj->Vertices)
		{
			if(obj->Vertices->vPos)
				free(obj->Vertices->vPos);
			free(obj->Vertices);
		}
		if(obj->Faces)
		{
			if(obj->Faces->face)
			{
				for(j=0; j<obj->Faces->n; j++)
					free(obj->Faces->face[j].vInd);

				free(obj->Faces->face);
			}
			free(obj->Faces);
		}
		
		if(obj->Molas)
		{
			if(obj->Molas->mola)
				free(obj->Molas->mola);
			free(obj->Molas);
		}
		
		if(obj->Parts)
		{
			if(obj->Parts->mvf)
				free(obj->Parts->mvf);
			free(obj->Parts);
		}

		free(obj);
	}
	return NULL;
}

void verficaColissao(umObjeto *obj) {
	vetVetexFace *faceColis= &(objFixo->Faces->face[1]); // vetor de indice dos vertices da face de colisao
	f4d *vPosVert = obj->Vertices->vPos;
	statePart *vPart = obj->Parts->mvf;
	f4d vPosPart, vVelN, vVelT;	// vetor de posicao da part. em relacao a um v�rtice da face de colisao
	int i;
	float s;
	for(i=0; i<obj->Parts->n; i++) {
			// vPosPart = posParticula - pontoFaceColisao
		vPosPart[0] = vPosVert[i][0] - objFixo->Vertices->vPos[faceColis->vInd[1]][0];
		vPosPart[1] = vPosVert[i][1] - objFixo->Vertices->vPos[faceColis->vInd[1]][1];
		vPosPart[2] = vPosVert[i][2] - objFixo->Vertices->vPos[faceColis->vInd[1]][2];
		
		s = calProdutoEscalar(faceColis->vNormal, vPosPart);
		if (s > 0.0)  {  //  os dois vetor em angulo obtuso
			// <<< existe colisao--> tratar >>>
			// resupera a posicao anterior
			vPosVert[i][0] = vPart[i].oldPos[0];
			vPosVert[i][1] = vPart[i].oldPos[1];
			vPosVert[i][2] = vPart[i].oldPos[2];
			// << mudan�a da velocidade da particula: normal em sentido contrario 
			// decompor a velocidade em normal (velN) e tangencial (velT) em relacao a face
			    // moduo da vel norma �: vn = vVel . vNormal
			s = calProdutoEscalar(vPart[i].vel, faceColis->vNormal);
				// vetor normal
			vVelN[0] = s * faceColis->vNormal[0]; 
			vVelN[1] = s * faceColis->vNormal[1];
			vVelN[2] = s * faceColis->vNormal[2];
				// vetor tangente
			vVelT[0] = vPart[i].vel[0] - vVelN[0];
			vVelT[1] = vPart[i].vel[1] - vVelN[1];
			vVelT[2] = vPart[i].vel[2] - vVelN[2];
				// aplicar restiticao e varia orientacao em vel normal
			vVelN[0] *= (-1.0 * gConsRest);
			vVelN[1] *= (-1.0 * gConsRest);
			vVelN[2] *= (-1.0 * gConsRest);
				// aplicar atrito na velocidade tangente
				//.... ignorar esta parte ...
				// obter a velocidade
			vPart[i].vel[0] = vVelN[0] + vVelT[0];
			vPart[i].vel[1] = vVelN[1] + vVelT[1];
			vPart[i].vel[2] = vVelN[2] + vVelT[2];
		}
	}
}

void computaForcas(umObjeto *obj) {
	int i, ia, ib, nPar, nMol;
	float s, ll;
	f4d vL, vI, ff;
	
	statePart *vPar;
	vetMola   *vMol;
	
	nPar = obj->Parts->n;
	vPar = obj->Parts->mvf;
	// forca de gravidade
	for(i=0; i<nPar; i++) {
		vPar[i].forc[0] = vPar[i].m * gGrav[0];
		vPar[i].forc[1] = vPar[i].m * gGrav[1];
		vPar[i].forc[2] = vPar[i].m * gGrav[2];
	}
	// forca das molas ---------------------------------------------------------------
	//     vL = Pos(a) - Pos(b) --> vetor de a para b
	//     vI = Vel(a) - Vel(b) --> vetor de velocidade de a , b
	//     r  = distancia de ropouso da mola (a, b) --> escalar.
	//     Ks = constante de elasticidade da mola
	//     Kd = constante de amortecimento da mola
	//  formula:
	//      fa = - [ Ks ( || vL || - r ) + Kd (vI . vL) / ||vL|| ] ( vL / ||vL||)
	//      fb = -fa
	// -------------------------------------------------------------------------------
	nMol = obj->Molas->n;
	vMol = obj->Molas->mola;
	for(i=0; i<nMol; i++) {
		ia = vMol[i].a;
		ib = vMol[i].b;
			// calcula vL 
		vL[0] = obj->Vertices->vPos[ia][0] - obj->Vertices->vPos[ib][0];
		vL[1] = obj->Vertices->vPos[ia][1] - obj->Vertices->vPos[ib][1];
		vL[2] = obj->Vertices->vPos[ia][2] - obj->Vertices->vPos[ib][2];		
			// calcula norma de vL
		ll = calNorma(vL);
		
			// calcula vetor unitario de vL em vL = vL/||vL||
		vL[0] /= ll;  vL[1] /= ll;  vL[2] /= ll;
		
			// cacula variacao de velocidade de a em relacao a b vI 
		vI[0] = vPar[ia].vel[0] - vPar[ib].vel[0];
		vI[1] = vPar[ia].vel[1] - vPar[ib].vel[1];
		vI[2] = vPar[ia].vel[2] - vPar[ib].vel[2];	
		
		s = calProdutoEscalar(vI, vL);          //  vI . ( vL / ||vL|| )
		s *= vMol[i].Kd;						//  Kd . ( vI . vL / ||vL|| )
		s += (vMol[i].Ks * (ll - vMol[i].r));	//  Ks ( || vL || - r ) + Kd (vI . vL) / ||vL|| 
		s *= -1.0;								// - [ Ks ( || vL || - r ) + Kd (vI . vL) / ||vL|| ]
		
		ff[0] = s * vL[0]; 
		ff[1] = s * vL[1];
		ff[2] = s * vL[2];	
		
			// fa = fa - ff
		vPar[ib].forc[0] -= ff[0];
		vPar[ib].forc[1] -= ff[1];
		vPar[ib].forc[2] -= ff[2];
			// fb = fb + ff
		vPar[ia].forc[0] += ff[0];
		vPar[ia].forc[1] += ff[1];
		vPar[ia].forc[2] += ff[2];			
	}	
}

static void passoEDOresolve(umObjeto *obj) {
	int i;
	static f4d  tmpVarPos, 	// temporal para calculo de variacao de posicao
				tmpVarVel;	// temporal para calculo de variacao de velociade
				
				
	vetPart *dinParts = obj->Parts;
	
	// fimpar for�a acumuladora de cada particula
	for(i=0; i<dinParts->n; i++) { 
		dinParts->mvf[i].forc[0] = 0.0; 
		dinParts->mvf[i].forc[1] = 0.0; 
		dinParts->mvf[i].forc[2] = 0.0;
	}
	
	computaForcas(obj);
	
	for(i=0; i<dinParts->n; i++) { 
		// -------------------( caso 1 )------------------------------
		// calcular Variacao Posicao: dPos * dt  ou seja  vel * dt
		// aqui... vel esta em dinParts->mvf.  dt � o mesmo gDt
		// -----------------------------------------------------
		
		//
		tmpVarPos[0] = dinParts->mvf[i].vel[0] * gDt;
		tmpVarPos[1] = dinParts->mvf[i].vel[1] * gDt;
		tmpVarPos[2] = dinParts->mvf[i].vel[2] * gDt;
		  

		//  
		
		// ----------------------( caso 2 )-----------------------------------
		// calcular Variacao Velocidade: dVel * dt --> acelera * dt
		//  guardar em tmpVarVel (este � variacao da velocidade) = acelerecao
		//  aqui...
		// -----------------------------------------------------
		
		
		// tmpVarVel = (forc/massa) * 
		tmpVarVel[0] = (dinParts->mvf[i].forc[0]/ dinParts->mvf[i].m) * gDt;
		tmpVarVel[1] = (dinParts->mvf[i].forc[1]/ dinParts->mvf[i].m) * gDt;
		tmpVarVel[2] = (dinParts->mvf[i].forc[2]/ dinParts->mvf[i].m) * gDt;
		  
		//
		
		// --------------------( caso 3 )-------------------------------------
		// calcular PosNova = Pos + variacaoPosicao
		// guardar em tmpVarPos = tempVarPos + Pos
		//   ou seja    tmpPos = Pos + tmpVarPos
		//  pos esta em obje->Vertices
		// ------------------------------------------------------------------
		
		//
		// tmpVarPos = temVarPos + pos;

		tmpVarPos[0] = tmpVarPos[0] + obj->Vertices->vPos[i][0]; 
		tmpVarPos[1] = tmpVarPos[1] + obj->Vertices->vPos[i][1];  
		tmpVarPos[2] = tmpVarPos[2] + obj->Vertices->vPos[i][2]; 

	//
	
		// --------------------( caso 4 )----------------------------------
		// calcular Veloc = Veloc + variacaoVelocidade
		// equivale a:  vel = vel + varVel
		// ------------------------------------------------------------------
		 //tmpVarVel = tmpVarVel + vel;

		tmpVarVel[0] = tmpVarVel[0] + dinParts->mvf[i].vel[0];
		tmpVarVel[1] = tmpVarVel[1] + dinParts->mvf[i].vel[1];
		tmpVarVel[2] = tmpVarVel[2] + dinParts->mvf[i].vel[2];


		// salvar velocidade atual como antigo (old)
		dinParts->mvf[i].oldPos[0] = obj->Vertices->vPos[i][0]; 
		dinParts->mvf[i].oldPos[1] = obj->Vertices->vPos[i][1];
		dinParts->mvf[i].oldPos[2] = obj->Vertices->vPos[i][2];
		
		// salvar velocidade atual como antigo (old)
		dinParts->mvf[i].oldVel[0] = dinParts->mvf[i].vel[0];
		dinParts->mvf[i].oldVel[1] = dinParts->mvf[i].vel[1];
		dinParts->mvf[i].oldVel[2] = dinParts->mvf[i].vel[2];
				
		// atualizar a posicao calculada
		obj->Vertices->vPos[i][0] = tmpVarPos[0]; 
		obj->Vertices->vPos[i][1] = tmpVarPos[1];
		obj->Vertices->vPos[i][2] = tmpVarPos[2];
		
		// salvar velocidade atual como antigo (old)
		dinParts->mvf[i].vel[0] = tmpVarVel[0];
		dinParts->mvf[i].vel[1] = tmpVarVel[1];
		dinParts->mvf[i].vel[2] = tmpVarVel[2];
		
	}	
}

static void animaObjeto(umObjeto *obj) {
	passoEDOresolve(obj);
	verficaColissao(obj);
}

int verExiteParNaLista(stLst *node, int a, int b) {
	int w = 0;  // nao existe
	printf("\n ver vertices (%d, %d) ", a, b);
	while(node) {
		if((node->a == a) && (node->b == b)) {
			w = 1;
			break;
		} else {
			if((node->a == b) && (node->b == a)) {
				w = 1;
				break;
			}
		}
		node = node->next;	
	}
	printf("- w = %d \n", w);
	return (w);	
}

void geraArestas(umObjeto *obj) 
{
	// gerando uma lista de molas.. saber quantos e quais s�o
	vetVetexFace *ff;
	stLst *first = NULL;
	stLst *node;
	int *vIndVert;
	int i, j, ii, m, mm, w, nn=0;
	
	ff = obj->Faces->face;
	m = obj->Faces->n;
	for(j=0; j<m; j++) {
		mm = ff[j].n;
		vIndVert = ff[j].vInd;
		for(i=0; i<mm; i++) {
			ii = (i+1)%mm;
			w = verExiteParNaLista(first, vIndVert[i], vIndVert[ii]);
			if(!w) {
				node = (stLst*) malloc(sizeof(stLst));
				node->a = vIndVert[i];
				node->b = vIndVert[ii];
				printf(" registrar: %d, %d ", node->a, node->b);
				// insere node na lista - metodo lifo
				node->next = first;
				first = node;
				nn++;
			}
		}
	}
	printf("\n numero de arestas em lista: %d \n", nn);
	
	// gerar o vetor de molas com nn elementos
	obj->Molas = (vetArest*) malloc(sizeof(vetArest));
	obj->Molas->mola = (vetMola*) malloc(nn*sizeof(vetMola));
	obj->Molas->n = nn;
	
	// copias elemtos da lista no vetor molas
	node = first;
	i = 0;
	while(node) {
		obj->Molas->mola[i].a = node->a;
		obj->Molas->mola[i].b = node->b;
		obj->Molas->mola[i].r = calDistancia(obj->Vertices->vPos[node->a], obj->Vertices->vPos[node->b]);
		obj->Molas->mola[i].Kd = gKd;	// dado global
		obj->Molas->mola[i].Ks = gKs;	// dado glogal
		node = node->next;
		i++;
	}
	
	// mostrar arestas
	for(i=0; i<obj->Molas->n; i++) 
		printf("\n (%d, %d), %4.2f, %4.2f, %4.2f ", obj->Molas->mola[i].a, obj->Molas->mola[i].b, obj->Molas->mola[i].r, obj->Molas->mola[i].Kd, obj->Molas->mola[i].Ks);

	// liberando lista de molas
	while(first) {
		node = first->next;
		free(first);
		first = node;
	}	
}

// ==== gerando particulas 
void geraParticula(umObjeto *obj) {
	int i, nn;
	
	nn = obj->Vertices->n;
	
	obj->Parts = (vetPart*) malloc (sizeof(vetPart));
	obj->Parts->n = nn;
	obj->Parts->mvf = (statePart*) malloc(nn*sizeof(statePart));
	for(i=0; i<nn; i++) {
		obj->Parts->mvf[i].m = gMassa;
		obj->Parts->mvf[i].vel[0] = 0.0; obj->Parts->mvf[i].vel[1] = 0.0; obj->Parts->mvf[i].vel[2] = 0.0;
		obj->Parts->mvf[i].forc[0] = 0.0; obj->Parts->mvf[i].forc[1] = 0.0; obj->Parts->mvf[i].forc[2] = 0.0;
	}		
}

// ==============================================================

void capturaObjeto(FILE *fobj, umObjeto *obj) {
	// carrega objeto do arquivo
	char token[40];
	float px, py, pz;
	int i, j, n, m;
	
	fgets(token, 40, fobj);  /* #cabecalho */  
	fscanf(fobj,"%s %d", token, &n);

	obj->Vertices = (vetPontos*) malloc(sizeof(vetPontos));

	obj->Vertices->n = n;
	obj->Vertices->vPos = (f4d*) calloc(n, sizeof(f4d));
	for(j=0; j<obj->Vertices->n; j++)
	{
		fscanf(fobj, "%s %f %f %f", token, &px, &py, &pz);
	 
		obj->Vertices->vPos[j][0] = px * local_scale;     
		obj->Vertices->vPos[j][1] = py * local_scale;
		obj->Vertices->vPos[j][2] = pz * local_scale;     
		obj->Vertices->vPos[j][3] = 0.0f;
	}

	fscanf(fobj,"%s %i", token, &n);

	obj->Faces = (vetFace*) malloc(sizeof(vetFace));

	obj->Faces->n = n;
	obj->Faces->face = (vetVetexFace*) calloc(n, sizeof(vetVetexFace));
	for(j=0; j<obj->Faces->n; j++)
	{
		fscanf(fobj, "%s %i", token, &m);
		obj->Faces->face[j].n = m;
		obj->Faces->face[j].vInd = (int*) calloc(m, sizeof(int));

		for(i=0; i<obj->Faces->face[j].n; i++)
		{
			fscanf(fobj, "%i", &m);
			obj->Faces->face[j].vInd[i] = m;
		}
	}
}

//  getting data

int procSolido(char *arch)
{
	FILE *fobj;

	if((fobj=fopen(arch,"rt"))==NULL)
	{
		printf("Error en la apertura del archivo %s \n", arch);
		return 0; 
	}
   
	if (objeto) objeto = liberaObjeto(objeto);

	if((objeto = criaObjeto())==NULL)
	{
		printf("\n Error en alocacion de memoria para un_objeto");
		return 0;
	}

	capturaObjeto(fobj, objeto);
	fclose(fobj);
  
	geraParticula(objeto);
	geraArestas(objeto);
  
	return 1;
}

int defineObjetoFixo (char *arch) {
	FILE *ff;
	int j, i;

	if(!objFixo) {
		if((ff=fopen(arch,"rt"))==NULL)
		{
			printf("Error en la apertura del archivo %s \n", arch);
			return 0; 
		}

		objFixo = criaObjeto();

		capturaObjeto(ff, objFixo);
		
		fclose(ff);
		
		for(j=0; j<objFixo->Faces->n; j++)
			calculaNormalFace(&(objFixo->Faces->face[j]), objFixo->Vertices); // computa normal da face
		
		// colocando na posicao certa (como base)
 		for(j=0; j<objFixo->Vertices->n; j++)
  		{ 
    		objFixo->Vertices->vPos[j][0] += gvPosBase[0];     
    		objFixo->Vertices->vPos[j][1] += gvPosBase[1];
    		objFixo->Vertices->vPos[j][2] += gvPosBase[2];     
		}
		// mostrar faces
		for(j=0; j<objFixo->Faces->n; j++) {
			printf("\n face %d: ", j);
			printf(" normal: (%4.2f, %4.2f, %4.2f) \n", objFixo->Faces->face[j].vNormal[0], objFixo->Faces->face[j].vNormal[1], objFixo->Faces->face[j].vNormal[2]);
			for(i=0; i<objFixo->Faces->face[j].n; i++) {
				printf("v%d : ind: %d ", i, objFixo->Faces->face[j].vInd[i]);
			}	
		}
	}	
}

// ==========================================================

static void Init(void)
{
//	gvPosBase[0] = 0.0;   gvPosBase[1] = -20.0;   gvPosBase[2] = 0.0;
	
// This function is called only once before any other functions are called
	alpha=-20.0; beta=20.0; RendMode=1;
//	gGrav[0] = 0.0;    gGrav[1] = -9.8; gGrav[2] = 0.0;  // acelercao de gravidade 
	gAnima = 0;

// Setting up a point light source
	//Adicionamos aqui
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
void MakeShapes(void)
{
// Geometric objects (axes and surfaces) are defined here. 
// If the shaopes change in time, the time parameter controllling these changes can be
// defined and changed in function draw()

// Since the axes are to be drawn as solid lines, we disable lighting for them.
    glDisable(GL_LIGHTING);
	glBegin(GL_LINES);

	glColor4f(1.0, 0.0, 0.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(2*SIZE,0.0,0.0);

	glColor4f(0.0, 1.0, 0.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,2*SIZE,0.0);

	glColor4f(0.0, 0.0, 1.0, 1.0);
	glVertex3f(0.0,0.0,0.0);
	glVertex3f(0.0,0.0,2*SIZE);
	    
	glEnd();
	        
// The polygons are rendered differently depending on the selected rendering modes:
// which are wireframe, flat shading, and smooth shading.
// Notice how the edges visually disappear when smooth rendering is selected.
// Default material settings are used. You may define your own material properties.

	if (RendMode==1){
		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);	
		glColor4f(0.0, 0.0, 1.0, 1.0);
	}
	else {
		glEnable(GL_LIGHTING);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	
	if (objeto) {
		if(gAnima) {
			animaObjeto(objeto);
			DesenhaObjeto(objFixo);
		}
		DesenhaObjeto(objeto);
	}
} 


//============================================================
static void draw( void )
{
	glClearColor (0.87, 0.88, 0.28, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambiente);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_difuso);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_especular);

	glMaterialfv(GL_FRONT, GL_SHININESS, mat_brilho);

	glEnable(GL_COLOR_MATERIAL);   
    
 	glMatrixMode(GL_MODELVIEW);	
	glPushMatrix();
	
// Placement and rotation of the scene.
	glTranslatef(0.0,0.0,-5*SIZE);
	glRotatef(beta, 1.0, 0.0, 0.0);
	glRotatef(alpha, 0.0, 1.0, 0.0);
	MakeShapes();
	glFlush();			         
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

//============================================================
static void hotkey(unsigned char k, int x, int y)
{
// Here we are processing keyboard events.
   switch (k) 
   {
      case 27:
      	liberaObjeto(objeto);
      	liberaObjeto(objFixo);
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
}

//============================================================
static void sfunc(int k, int x, int y)
{
// Here we can process function keys and other special key events
   switch (k) 
   {

// Rotate to the left
	  case GLUT_KEY_LEFT:
	  alpha-=3.0;
	  break;

// Rotate to the right
	  case GLUT_KEY_RIGHT:
	  alpha+=3.0;
	  break;
   }
}
// ==========================================================


void processMenuEvents(int option) 
{
	gAnima = 0;
	if (option == 1)
		procSolido("dadosCubo.txt");  // data/dadosCubo.txt
	else if (option == 2)
		procSolido("dadosPiramide.txt");
	else if (option == 3) 
		gAnima = 1;
		
	defineObjetoFixo("dadosCuboBase.txt");
	
	glutPostRedisplay();
}

void createGLUTMenus() 
{
	int menu, submenu1;
	
	submenu1 = glutCreateMenu(processMenuEvents);
	glutAddMenuEntry("Solido Cubo",1);
	glutAddMenuEntry("Solido Piramide",2);
	glutAddMenuEntry("Animar",3);
	
	menu = glutCreateMenu(processMenuEvents);
	glutAddSubMenu("Objetos",submenu1);

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
   glutCreateWindow ("Sample OpenGL program.          \
   Use  -->,  <--,  <w>,  <f>,  and  <Esc> keys.");

   Init();
   glutReshapeFunc(reshape);
   glutIdleFunc(idle);
   glutDisplayFunc(draw);
   glutKeyboardFunc(hotkey);
   glutSpecialFunc(sfunc);

	createGLUTMenus(); 

// Refer to GLUT manual on how to set up and use pop-up menus.

// The main event loop is started here.
   glutMainLoop();
}
