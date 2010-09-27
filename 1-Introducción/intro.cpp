// Programa introductorio
// triángulo con tres puntos editables

#include <iostream> // cout
#include <cstdlib> // exit
#include <cmath> // fabs
#include <GL/glut.h>

using namespace std;

//------------------------------------------------------------
// variables globales
int
  w=480,h=360, // tamaño inicial de la ventana
  npuntos=0, // cantidad de puntos
  pt[8], // los puntos (hasta 3: x0,y0,x1,y1,x2,y2)
  ep=-1; // índice del punto a editar
int  xp, yp; //punto en el que clickea adentro del triangulo
GLubyte color_p[3]={120,128,200}; 
//============================================================
// callbacks

//------------------------------------------------------------

// arma un un nuevo buffer (back) y reemplaza el framebuffer
void Display_cb() {
  //static int counter=0; cout << "display: " << counter++ << endl;

  // arma el back-buffer
  glClearColor(0.0, 1.0, 1.0, 2.0); // clearing (background) color
  glClear(GL_COLOR_BUFFER_BIT);//  Clear all pixels and refill with background color

  if (!npuntos){ // no hay nada
    glutSwapBuffers(); // manda al monitor la pantalla vacía
    return;
  }

  // dibuja
  int i;
  // triángulo (sólo si ya están los tres puntos)
  if (npuntos==4) {
    
    glColor3ubv(color_p);
    
    
    glBegin(GL_QUADS);
    for(i=0;i<npuntos-2;i++){
           
      glVertex2i(pt[2*i],pt[2*i+1]);
    }
    int v0x=pt[2]-pt[0], 
      v0y=pt[3]-pt[1],
      v1x=pt[4]-pt[0],
      v1y=pt[5]-pt[1],
      v2x=pt[6]-pt[0],
      v2y=pt[7]-pt[1];
    float v0xv1=(v0x*v1y)-(v0y*v1x), v1xv2=(v1x*v2y)-(v1y*v2x);
    if (v0xv1*v0xv1>0)
    {
      glVertex2i(pt[2*i],pt[2*i+1]);
      i+=1;
      glVertex2i(pt[2*i],pt[2*i+1]);
    }
    else
      {
        i+=1;
        glVertex2i(pt[2*i],pt[2*i+1]);
        i-=1;
        glVertex2i(pt[2*i],pt[2*i+1]);
      }
    
    glEnd();
    
    glColor3f(.12,.3,.1);
    glLineWidth(8.0);
    glBegin(GL_LINE_LOOP);
    for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]); //funcionamiento dinamico, muevo el punto y va cambiando la coordenada
    glEnd();       
    
  }
   
  
  
  // puntos (después del triángulo, para que se vean encima)
  glColor3f(.0f,.0f,.0f);
  glPointSize(6.0); // punto grueso
  glBegin(GL_POINTS);
  for(i=0;i<npuntos;i++) glVertex2i(pt[2*i],pt[2*i+1]);
  glEnd();
  
  
   
  
  glutSwapBuffers(); // lo manda al monitor
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  cout << "reshape " << width << "x" << height << endl;
  if (!width||!height) return; // minimizado ==> nada
  w=width; h=height;
  glViewport(0,0,w,h); // región donde se dibuja (toda la ventana)
  // rehace la matriz de proyección (la porcion de espacio visible)
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();
  glOrtho(0,w,0,h,-1,1); // unidades = pixeles
  Display_cb(); // Redibuja mientras hace el reshape!!
}

//------------------------------------------------------------
// Mouse

// Drag (movimiento con algun boton apretado)
void Motion_cb(int x, int y){
  y=h-y; // initially zero is in the upper-left corner, with this, zero is in the lower-left corner 
  cout << x << "," << y << endl;
  if(ep=-2){
    for (int i=0;i<npuntos;i++){
      pt[2*i]+=(x-xp);pt[2*i+1]+=(y-yp);
    }
    xp=x;yp=y;
  } 
  else
  if(x>0 && x<w && y>0 && y<h){ 
  pt[2*ep]=x;pt[2*ep+1]=y; // fija el punto editado en x,y
  //Display_cb(); // Redibuja mientras va moviendo
  }
  glutPostRedisplay();
}

// Botones picados o soltados
void Mouse_cb(int button, int state, int x, int y){
  
  cout << x << "," << y << endl;
  if (button==GLUT_LEFT_BUTTON){ // boton izquierdo
    if (state==GLUT_DOWN) { // clickeado
      y=h-y; // el 0 está arriba
      //cout << x << "," << y << endl;
      ep=-1;
      // verifica si picó a menos de 5 pixeles de algún punto previo
      for (int i=0;i<npuntos;i++){
        int d=abs(x-pt[2*i])+abs(y-pt[2*i+1]); // distancia simple (métrica del rombo)
        if (d>5) continue; // lejos
        ep=i; // edita el punto i
        glutMotionFunc(Motion_cb); // define el callback para los drags
        return;
      }
     if(npuntos==4){
      GLubyte color[3];
            
      glReadBuffer(GL_FRONT);
      glReadPixels(x,y,1,1,GL_RGB,GL_UNSIGNED_BYTE,color);
      if(color[0]==color_p[0] && color[1]==color_p[1] && color[2]==color_p[2])
      {
        xp=x; yp=y; ep=-2;
        glutMotionFunc(Motion_cb);
      }
     }   // no pico cerca de otro
      if (npuntos==4) return; // ya hay 3 ==> no hace nada
      // agrega un punto
      pt[2*npuntos]=x; pt[2*npuntos+1]=y; npuntos++;
      Display_cb(); // Redibuja
      // y queda listo para editarlo hasta que suelte el botón izquierdo
      ep=npuntos-1; // edita el ultimo punto
      glutMotionFunc(Motion_cb); // define el callback para los drags
    } // fin clickeado
    else if (state==GLUT_UP) // soltado
      glutMotionFunc(0); // anula el callback para los drags
  } // fin botón izquierdo
}

//------------------------------------------------------------
// Teclado

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea (aqui no importan)
void Keyboard_cb(unsigned char key,int x,int y) {
 
  if (key==27){
    npuntos=0;
    //Display_cb();
    glutPostRedisplay();
    //glClear(GL_COLOR_BUFFER_BIT);
   // glutSwapBuffers();
  }
  
  if (key==127||key==8){ // DEL o Backspace ==> borra el punto ep (el editable)
    if (ep==-1||ep==npuntos) return;
    // corre los siguientes hacia atras
    for(int i=ep;i<npuntos-1;i++) {pt[2*i]=pt[2*i+2];pt[2*i+1]=pt[2*i+3];}
    npuntos--;
    Display_cb(); // Redibuja
    // Backspace borra para atras, DEL borra para adelante
    if (key==8) ep--; // ep pasa a ser el anterior
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
  glutInitDisplayMode(GLUT_RGB|GLUT_DOUBLE);// pide color RGB y double buffering
  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);
  glutCreateWindow("introducción"); // crea el main window

  //declara los callbacks, los que (aun) no se usan (aun) no se declaran
  glutDisplayFunc(Display_cb);
  glutReshapeFunc(Reshape_cb);
  glutKeyboardFunc(Keyboard_cb);
  glutSpecialFunc(Special_cb);
  glutMouseFunc(Mouse_cb);

  // OpenGL
  glClearColor(0.85f,0.9f,0.95f,1.f); // color de fondo
  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // constante
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv); // inicialización interna de GLUT
  inicializa(); // define el estado inicial de GLUT y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos. Once this loop is entered, it is never exited. Pag 19
  return 0; // nunca se llega acá, es sólo para evitar un warning
}
