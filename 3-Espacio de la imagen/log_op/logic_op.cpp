// Programa Operaciones Logicas

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360, // tamaño inicial de la ventana
  npuntos=3, // cantidad de puntos
  pt[6]={120,100,360,100,240,260}, // los puntos (hasta 3: x0,y0,x1,y1,x2,y2)
  xini,yini,xfin,yfin;

//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  int i;
  static int count=0;
  cout << "display: " << ++count <<endl;
  
  // arma el color-buffer
  glClear(GL_COLOR_BUFFER_BIT);// rellena con color de fondo

  // dibuja el triángulo
  glColor3f(1.0f,0.f,0.f);
  glBegin(GL_TRIANGLES);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();
  
  // puntos (después del triángulo, para que se vean encima)
  glColor3f(0.f,0.f,1.0f);
  glPointSize(5.0); // punto grueso
  glBegin(GL_POINTS);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();
  
  //bool habilitado=glIsEnabled(GL_COLOR_LOGIC_OP);
  glFlush(); // lo manda al monitor
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  // cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1); // unidades = pixeles
  Display_cb(); // Redibuja mientras hace el reshape
}

//------------------------------------------------------------
// Mouse

// Drag (movimiento con algun boton apretado)
void Motion_cb(int x, int y){
  // Dibuja la linea vieja (borra)
  glBegin(GL_LINE_LOOP);
    glVertex2i(xini,yini); glVertex2i(xfin,yini);
    glVertex2i(xfin,yini); glVertex2i(xfin,yfin);
    glVertex2i(xfin,yfin); glVertex2i(xini,yfin);
  glEnd();
  
  xfin=x;
  yfin=h-y;
  // Dibuja la linea nueva
  glBegin(GL_LINE_LOOP);
    glVertex2i(xini,yini); glVertex2i(xfin,yini);
    glVertex2i(xfin,yini); glVertex2i(xfin,yfin);
    glVertex2i(xfin,yfin); glVertex2i(xini,yfin);
  glEnd();
  glFlush();
}

// Botones picados o soltados
void Mouse_cb(int button, int state, int x, int y){
  if(button==GLUT_LEFT_BUTTON){ // boton izquierdo
    if(state==GLUT_DOWN){ // clickeado
	    GLint factor=1;GLushort patron=0xFF00;//0x5555 0xFF00 0x3333 0x7733
	    glLineStipple(factor,patron);
   	  glEnable(GL_LINE_STIPPLE); // lineas de trazos
      glLogicOp(GL_INVERT); // invierte el color
      glEnable(GL_COLOR_LOGIC_OP); // habilita las operaciones logicas 
      glutMotionFunc(Motion_cb); // define el callback para los drags
      xini=xfin=x;
      yini=yfin=h-y;
    } // fin clickeado
    else if (state==GLUT_UP){ // soltado
      // PROBLEMA: freeglut llama al display callback al soltar el boton (???)
      /*
      // glut
      // Dibuja la linea vieja (borra)
      glBegin(GL_LINE_LOOP);
        glVertex2i(xini,yini); glVertex2i(xfin,yini);
        glVertex2i(xfin,yini); glVertex2i(xfin,yfin);
        glVertex2i(xfin,yfin); glVertex2i(xini,yfin);
      glEnd();
      glFlush();
      */
      glDisable(GL_COLOR_LOGIC_OP);
      glDisable(GL_LINE_STIPPLE);
      glutMotionFunc(0); // anula el callback para los drags
      // freeglut:
      glutPostRedisplay(); // que dibuje de nuevo (al cuete!)
    } // fin soltado
  } // fin botón izquierdo
}

//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea (aqui no importan)
void Keyboard_cb(unsigned char key,int x,int y) {
  if (key==27){
    exit(EXIT_SUCCESS);
  }
}

// Special keys (non-ASCII)
// teclas de funcion, flechas, page up/dn, home/end, insert
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4 && glutGetModifiers()==GLUT_ACTIVE_ALT) // alt+f4 => exit
    exit(EXIT_SUCCESS);
}

//------------------------------------------------------------
void inicializa() {
  // GLUT
  glutInitDisplayMode(GLUT_RGB|GLUT_SINGLE);// pide color RGB y double buffering GLUT_DOUBLE
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("Operaciones lógicas"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);
  //glutMotionFunc(Motion_cb);

  // OpenGL
  //glClearColor(0.85f,0.9f,0.95f,1.f); // color de fondo
  glClearColor(0.9f,0.9f,0.9f,1.f); // color de fondo
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
