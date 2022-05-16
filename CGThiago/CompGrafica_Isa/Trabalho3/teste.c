#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "GL/glut.h"

#define SENS_ROT 10.0 /*Define = Valor constante em C*/
#define SENS_OBS 10.0
#define SENS_TRANS 10.0

#define FORCA 10
#define GRAVIDADE 9.9999

#define MAXPARTICULAS  10000
#define randomico() ((float) rand()/ RAND_MAX) //Numero aleatório entre 0 e1

GLfloat ASPECTO, ANGULO;
GLfloat obsX, obsY, obsZ, rotX, rotY;
GLfloat obsX_ini, obsY_ini, obsZ_ini, rotX_ini, rotY_ini;
GLfloat escalaX, escalaY, escalaZ;
int x_ini,y_ini, bot;
int origemX, origemY;
float angle = 36;


//PARTÍCULAS=======================================================================================
typedef struct{
        GLfloat posicao2[3];
        GLfloat velocidade2[3];
        GLfloat aceleracao2[3];
        GLfloat massa;
        GLfloat forca;
        GLfloat cor2[3];
        GLfloat tempoVida2;
        GLfloat transparencia2;
}part2;

part2 Particulas2[MAXPARTICULAS];

void conceberParticulas2 (int i){     
     //GLfloat r, alpha;//Raio, Angulo
     GLfloat alpha2, beta2;//Angulos, raio         
     GLfloat raio2 = 0.008 * randomico() + 0.002;
     alpha2 = 2 * M_PI * randomico();
     beta2 = M_PI * randomico();
     float raio = 2;
     //float raio = 0.1;
     //float teta = 0.5  * randomico() + 0.002;
           
     //Coordenadas do ponto de origem da partículas    
     Particulas2[i].posicao2[0] = raio*cos(angle);
     Particulas2[i].posicao2[1] = 5.0;   //y 10.5
     Particulas2[i].posicao2[2] = raio*sin(angle);
     
     Particulas2[i].velocidade2[0]=  raio2 * cos(alpha2) * sin(beta2);//x
     Particulas2[i].velocidade2[1]=  0;
     Particulas2[i].velocidade2[2]=  raio2 * sin(alpha2) * sin(beta2);//z  

     Particulas2[i].massa = 1;

     Particulas2[i].forca = FORCA - (Particulas2[i].massa*GRAVIDADE); 
     
     Particulas2[i].aceleracao2[0] = Particulas2[i].forca/Particulas2[i].massa; //x
     Particulas2[i].aceleracao2[1] = 0.0;//y 
     Particulas2[i].aceleracao2[2] = Particulas2[i].forca/Particulas2[i].massa;//z
     
     Particulas2[i].cor2[0] = randomico();//x
     Particulas2[i].cor2[1] = 0.1*randomico();//y 
     Particulas2[i].cor2[2] = 0.01*randomico();;//z  
     
     Particulas2[i].tempoVida2 = 0.5 + 0.5 * randomico();
     Particulas2[i].transparencia2 = 1.0;   
     angle+= 36;
}/*Fim conveberParticulas*/

void extinguirParticulas2 (int i){
     if(Particulas2[i].tempoVida2 < 0.01){     
        conceberParticulas2(i);                            
     } 
}/*Fim extinguirParticulas*/

void iniciaParticulas2 (void){
     int i;
    //origemX = rand() % 8 - 4;
    //origemY = (1 + (rand() % 10 ))/10;
     for(i = 0; i< MAXPARTICULAS; i++){
        conceberParticulas2(i);
     }
}

void posicionaObservador (void) {
     glMatrixMode (GL_MODELVIEW);/*Coordenadas na matrix de visualização*/  
     glLoadIdentity();
     //Posiciona e orienta o observador
     glTranslatef(-obsX, -obsY, -obsZ);/*Translata a câmera para essas variáveis*/
     glRotatef(rotX,1,0,0);/*Rotacionar a câmera para essas coordenadas*/
     glRotatef(rotY,0,1,0);   
}/*Fim posicionaObservador*/

void motion(int x, int y){
     int deltaX, deltaY;
     if(bot == GLUT_LEFT_BUTTON){//Rotação
         deltaX = x_ini - x;
         deltaY = y_ini - y;
         
         rotX = rotX_ini - deltaY/ SENS_ROT;
         rotY = rotY_ini - deltaX/ SENS_ROT;
     }
     else if (bot == GLUT_RIGHT_BUTTON){//Zoom
             int deltaZ = y_ini - y;
             obsZ = obsZ_ini + deltaZ/ SENS_OBS;
     }
     else if (bot == GLUT_MIDDLE_BUTTON){//Correr
          deltaX = x_ini - x;
          deltaY = y_ini - y;
          obsX = obsX_ini + deltaX/ SENS_TRANS;
          obsY = obsY_ini + deltaY/ SENS_TRANS;
          }
     posicionaObservador();
     glutPostRedisplay();
     
}

void especificaParametrosVisuais (void){
     glMatrixMode(GL_PROJECTION);/*Modo de visualização da matriz, Projeção*/
     glLoadIdentity();
                      
     gluPerspective (ANGULO, ASPECTO, 0.5, 500);     
     
     posicionaObservador();
}/*Fim especificaParametrosVisuais*/
     
void redesenhaPrimitivas(GLsizei largura, GLsizei altura){ 
     if (altura == 0)
          altura = 1;
     glViewport (0, 0, largura, altura);/*Dimensiona o ViewPort*/
     
     ASPECTO = (GLfloat) largura/ (GLfloat) altura;/*Calcula a correção de aspecto*/ 
     
     especificaParametrosVisuais();        
}/*Fim redesenhaPrimiti*/

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
}/*Fim mouse*/

void inicializa(void){
     glClearColor (0.0f, 0.0f, 0.0f, 1.0f);
     //glClearColor (0.5f, 0.7f, 1.0f, 1.0f);
     ANGULO = 45;
     rotX = rotY = 0;
     obsX = obsY = 1;
     obsZ = 40;//Voltar para 10
     
     escalaX = escalaY = escalaZ = 1;
     iniciaParticulas2(); 
}

void disco(){
     GLUquadricObj* q = gluNewQuadric();
     gluQuadricDrawStyle(q, GLU_FILL);
     gluQuadricNormals(q, GLU_SMOOTH);
     gluQuadricTexture(q, GL_TRUE);
     gluCylinder(q, 0, 2, 0.0, 60, 60);
     gluDeleteQuadric(q);
}

void desenhaPlano(){
    // glClear limpa tambem o buffer de profundidade
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glColor3f( 0.5, 0.5, 0.5 );          // Desenha na cor cinza
    glMatrixMode( GL_MODELVIEW );
    
   // glutSolidCube( 10.0 );                  // desenha cubo com tamanho igual a 10 
    
    glFlush();  
    glDisable(GL_TEXTURE_2D);
     
    glPushMatrix();    //Superior
    glColor3f(0.5f, 0.7f, 0.6f);                                                                                                                    
    glTranslatef (0.0, 5.0, 0.0);// (x,y,z)
    glRotatef (-90.0, 1.0, 0.0, 0.0);// (Angulo,x,y,z)
    // glScalef (escalaX , escalaY -0.5, escalaZ);// (Escala)  
    disco();/*AQUI que desenha pela quádrica*/     
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);     
}

//DESENHO============================================================================================================
void desenhaParticula2(void){
     int i;
     float dTempo = 1.0;
     //glClear(GL_COLOR_BUFFER_BIT);     
     glDisable(GL_DEPTH_TEST);//"Rastro"
     glEnable(GL_BLEND);//Habilita a transparência
     glBlendFunc(GL_SRC_ALPHA, GL_ONE);//Ativa a Transparência
     glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);//Perspectiva    
     
    for(i = 0; i<MAXPARTICULAS; i++){
             glColor4f(Particulas2[i].cor2[0], Particulas2[i].cor2[1], Particulas2[i].cor2[2], Particulas2[i].transparencia2);             
             glPointSize(2.0);
             glPushMatrix();
             glBegin(GL_POINTS);
                 glVertex3f(Particulas2[i].posicao2[0], Particulas2[i].posicao2[1], Particulas2[i].posicao2[2]);
             glEnd();
             glPopMatrix();
             //Se for para ser estático Não tem as linhas abaixo!
             Particulas2[i].posicao2[0] +=  dTempo*Particulas2[i].velocidade2[0];
             Particulas2[i].posicao2[1] +=  dTempo*Particulas2[i].velocidade2[1];
             Particulas2[i].posicao2[2] +=  dTempo*Particulas2[i].velocidade2[2];
             
             //soma das forças m.g + fn = fr = ma ->> a = fr/m

             Particulas2[i].velocidade2[0] += dTempo*Particulas2[i].aceleracao2[0];
             Particulas2[i].velocidade2[1] += dTempo*Particulas2[i].aceleracao2[1];
             Particulas2[i].velocidade2[2] += dTempo*Particulas2[i].aceleracao2[2];
             
             Particulas2[i].tempoVida2 -= 0.01;
             Particulas2[i].transparencia2 -= 0.01;
             extinguirParticulas2(i);
    }
     glDisable(GL_BLEND);
     //glutSwapBuffers();
}

//DESENHA TUDO========================================================================================================
void desenhaTudo(void){
     glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);/*Relacionada ao buffer de renderização*/
     desenhaPlano();
     desenhaParticula2();          
     glutSwapBuffers();
}

void idleF (void){
     glutPostRedisplay();
}/*Fim Idle*/

//********************************************************************************************
int main(int argc, char** argv){/*Rotina Principal da aplicação*/
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);/*Função de inicialização da tela*/
    glutInitWindowPosition(3, 3);/*Função de inicialização da tela*/
    glutInitWindowSize(1000, 1000);/*Função de renderização*/
    glutCreateWindow("Sistema de Particulas");/*Criação propriamente dita e nomeação da janela*/
    
    glutDisplayFunc(desenhaTudo);/*Desenho na tela*/
    glutReshapeFunc(redesenhaPrimitivas);/*Redesenho na tela*/
    glutMouseFunc(mouse);/*Rotina do mouse*/
    glutMotionFunc(motion);/*Rotina do movimento*/
    glutIdleFunc(idleF); 
    inicializa();/*Constante a cor da tela*/
           
    glutMainLoop();
    return 0;
}/*Fim do MAIN*/
