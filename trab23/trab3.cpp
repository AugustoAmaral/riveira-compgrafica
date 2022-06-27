#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "GL/glut.h"

#define SENS_ROT 10.0   // Define = Valor constante em C
#define SENS_OBS 10.0
#define SENS_TRANS 10.0

#define PASSO_TEMP 0.1  // passo de tempo para integrar ODE por Euler 


#define MAXPARTICULAS  10000
#define randomico() ((float) rand()/ RAND_MAX) //Numero aleatorio entre 0 e 1

GLfloat ASPECTO, ANGULO;
GLfloat obsX, obsY, obsZ;               // posicao do observador (obsX, obsY, obsZ) 
GLfloat rotX, rotY;                     // Angulos de Orientacao x , y (rotX, rotY) na naiz do observador
GLfloat obsX_ini, obsY_ini, obsZ_ini;  // caso de transformacao (posicao inicia para se obter nova posicao)
GLfloat rotX_ini, rotY_ini;            // angulos de orientacao inicial para transformacao 
GLfloat escalaX, escalaY, escalaZ;     // fatores de escala para trasnformacao

int x_ini,y_ini, bot;
int origemX, origemY;

//PARTÍCULAS=======================================================================================
typedef struct{
        GLfloat pos[3];
        GLfloat vel[3];
        GLfloat ace[3];
        GLfloat massa;
        GLfloat cor[3];
        GLfloat tempoVida;
        GLfloat transparencia;
}part2;

part2 Particulas[MAXPARTICULAS];

void conceberParticulas (int i){     
     //GLfloat r, alpha;   //Raio, Angulo
     GLfloat alpha, beta;                              
     GLfloat raio = 0.1 * randomico() + 0.06;
     alpha = 2 * M_PI * randomico();
     beta = M_PI * randomico();
      
     //Coordenadas do ponto de origem da particulas    
     Particulas[i].pos[0] = rand() % 8 - 4;;   // posicao em x
     Particulas[i].pos[1] = 5.0;               // posicao em y
     Particulas[i].pos[2] = rand() % 2 - 2;    // posicao em z
     
     Particulas[i].vel[0]=  raio * cos(alpha) * sin(beta);  // velocidade em x
     Particulas[i].vel[1]=  raio * cos(beta);               // velocidade em y
     Particulas[i].vel[2]=  raio * sin(alpha) * sin(beta);  // velocidade em z   
     
     Particulas[i].ace[0] = 0.0;    // acelera em x
     Particulas[i].ace[1] = 0.025;  // acelera em y 
     Particulas[i].ace[2] = 0.0;    // acelera em z
     
     Particulas[i].massa = 0.01*randomico();  // massa da particula
     
     Particulas[i].cor[0] = randomico();        // R
     Particulas[i].cor[1] = 0.1*randomico();    // G 
     Particulas[i].cor[2] = 0.01*randomico();;  // B  
     
     Particulas[i].tempoVida = 0.8 + 0.98 * randomico();  // define o tempo de vida da particula
     Particulas[i].transparencia = 1.0;                  // grau de transaperencia da particula
}

void extinguirParticulas (int i){
     if(Particulas[i].tempoVida < 0.001){     
        conceberParticulas(i);                            
     } 
}

void iniciaParticulas (void){
     int i;

     for(i = 0; i< MAXPARTICULAS; i++){
        conceberParticulas(i);
     }
}

void posicionaObservador (void) {
     glMatrixMode (GL_MODELVIEW);  //Coordenadas na matrix de visualizacao
     glLoadIdentity();
     
     //Posiciona e orienta o observador
     glTranslatef(-obsX, -obsY, -obsZ);  // Translata a camera para essas variaveis
     glRotatef(rotX,1,0,0);      // Rotacionar a camera para essas coordenadas
     glRotatef(rotY,0,1,0);   
}

void motion(int x, int y){
     int deltaX, deltaY;
     if(bot == GLUT_LEFT_BUTTON){//Rotação
         deltaX = x_ini - x;
         deltaY = y_ini - y;
         
         rotX = rotX_ini - deltaY/ SENS_ROT;
         rotY = rotY_ini - deltaX/ SENS_ROT;
     }
     else if (bot == GLUT_RIGHT_BUTTON){ //Zoom
             int deltaZ = y_ini - y;
             obsZ = obsZ_ini + deltaZ/ SENS_OBS;
     }
     else if (bot == GLUT_MIDDLE_BUTTON){ //Correr
          	deltaX = x_ini - x;
          	deltaY = y_ini - y;
          	
          	obsX = obsX_ini + deltaX/ SENS_TRANS;
          	obsY = obsY_ini + deltaY/ SENS_TRANS;
          }
     posicionaObservador();
     glutPostRedisplay();
     
}

void especificaParametrosVisuais (void){
     glMatrixMode(GL_PROJECTION);
     glLoadIdentity();
                      
     gluPerspective (ANGULO, ASPECTO, 0.5, 500);     
     
     posicionaObservador();
}
     
void redesenhaPrimitivas(GLsizei largura, GLsizei altura){ 
     if (altura == 0)
          altura = 1;
          
     glViewport (0, 0, largura, altura);  // defina tamanho de viewport - canvas
     
     ASPECTO = (GLfloat) largura/ (GLfloat) altura; // calcula acorrecao de aspecto 
     
     especificaParametrosVisuais();        
}

void mouse(int botao, int estado, int x, int y){
     if(estado == GLUT_DOWN){
         x_ini = x;
         y_ini = y;
         
         obsX_ini = obsX;
         obsY_ini = obsY;
         obsZ_ini = obsZ;
         
         rotX_ini = rotX;
         rotY_ini = rotY;
         
         bot=botao;
     }
     else
         bot = -1;
}

void inicializa(void){
     glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
   
     ANGULO = 45;
     
     rotX = 0;
	 rotY = 0;
	 
     obsX = 1;
	 obsY = 1;
     obsZ = 40;
     
     escalaX = 1;
	 escalaY = 1;
	 escalaZ = 1;
	 
     iniciaParticulas(); 
}

void disco(GLfloat raioInf, GLfloat raioSup, GLfloat alt){
     GLUquadricObj* q = gluNewQuadric();
     gluQuadricDrawStyle(q, GLU_FILL);
     gluQuadricNormals(q, GLU_SMOOTH);
     gluQuadricTexture(q, GL_TRUE);
     gluCylinder(q, raioInf, raioSup, alt, 60,60);
     gluDeleteQuadric(q);
}

void desenhaPlano(){
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glColor3f( 0.5, 0.5, 0.5 );          // Desenha na cor cinza
    glMatrixMode( GL_MODELVIEW );
    
    glutWireCube( 10.0 );                // desenha cubo com tamanho igual a 10 
    
    glFlush();  
    glDisable(GL_TEXTURE_2D);
     
    glPushMatrix();               // empilha a matriz de operacao
    glColor3f(0.5f, 0.7f, 0.6f);                                                                                                                    
    glTranslatef (0.0, 5.0, 0.0);      // Translada (x,y,z)
    glRotatef (-90.0, 1.0, 0.0, 0.0);   // Rotaciona (1,0,0) en Angulo
    glScalef (escalaX , escalaY -0.5, escalaZ);    // Escala 
    disco(4.0, 0.0, 0.0);   // gera um disco (cilinro) de raio inferio 4, raio sup 0 e altura 0  
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);     
}

//         DESENHO
// -------------------------
void desenhaParticula2(void){
     int i;
    
     glDisable(GL_DEPTH_TEST);            //"Rastro"
     glEnable(GL_BLEND);                  //Habilita a transparencia
     glBlendFunc(GL_SRC_ALPHA, GL_ONE);   //Ativa a Transparência
     glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);    //Perspectiva    
     
    for(i = 0; i<MAXPARTICULAS; i++){
        glColor4f(Particulas[i].cor[0], Particulas[i].cor[1], Particulas[i].cor[2], Particulas[i].transparencia);             
        glPointSize(2.0);
        glPushMatrix();
        glBegin(GL_POINTS);
            glVertex3f(Particulas[i].pos[0], Particulas[i].pos[1], Particulas[i].pos[2]);
        glEnd();
        glPopMatrix();
             
            // calculando EDO com Euler
        Particulas[i].pos[0] +=  PASSO_TEMP * Particulas[i].vel[0];
        Particulas[i].pos[1] +=  PASSO_TEMP * Particulas[i].vel[1];
        Particulas[i].pos[2] +=  PASSO_TEMP * Particulas[i].vel[2];
             
        Particulas[i].vel[0] += PASSO_TEMP * Particulas[i].ace[0];
        Particulas[i].vel[1] += PASSO_TEMP * Particulas[i].ace[1];
        Particulas[i].vel[2] += PASSO_TEMP * Particulas[i].ace[2];
             
        Particulas[i].tempoVida -= 0.01;
        Particulas[i].transparencia -= 0.01;
        extinguirParticulas(i);
    }
    glDisable(GL_BLEND);
}

//    DESENHA TUDO  

void desenhaTudo(void){
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
     desenhaPlano();
     desenhaParticula2();          
     glutSwapBuffers();
}

void idleF (void){
     glutPostRedisplay();
}


int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);
    glutInitWindowPosition(3, 3);
    glutInitWindowSize(1000, 1000);
    glutCreateWindow("Sistema de Particulas");
    
    glutDisplayFunc(desenhaTudo);
    glutReshapeFunc(redesenhaPrimitivas);
    glutMouseFunc(mouse);
    glutMotionFunc(motion);
    glutIdleFunc(idleF);   // fun?ão de animacao
    inicializa();
           
    glutMainLoop();
    return 0;
}