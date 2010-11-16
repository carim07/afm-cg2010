#include <cmath> // sqrt
#include <cstdlib> // exit (si va, va necesariamente antes del include de glut)
#include <iostream> // cout
#include <GL/glut.h>
#include <fstream>
#include "Maths/Maths.h" // Librerias con implementacion de vectores y matrices de paulsprojects.net
using namespace std;

//------------------------------------------------------------
// VENTANA DE CONSOLA (salida de texto via cout)
// En linux depende de si se ejecuta sobre una consola o directamente.
// En windows, si no se quiere la ventana de consola para cout, se debe 
//    armar un proyecto windows (en linux o Windows) en lugar de console
// Visual
//   Basta con descomentar este bloque que sigue para sacar la consola
/*
#ifdef _MSC_VER
 #pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )
#endif
*/
//   La alternativa es ir a las propiedades de link y cambiarlo a mano:
// /subsystem:console --> /susystem:windws /entry:mainCRTStartup
// Mingw (gcc) -mwindows en lugar del default -mconsole
//------------------------------------------------------------

// variables globales y defaults

int
  w=480,h=360, // tama�o de la ventana
  boton=-1, // boton del mouse clickeado
  xclick,yclick, // x e y cuando clickeo un boton
  lod=16; // nivel de detalle (subdivisiones de lineas y superficies parametricas)
float // luces y colores en float
//  lambient[]={.2f,.2f,.2f,1}, // luz ambiente
//  ldiffuse[]={.7f,.7f,.7f,1}, // luz difusa
  fondo[]={.9f,.9f,.9f,1},    // color de fondo
  face_color[]={.4f,.5f,.6f,1},    // color de caras
  line_color[]={.2f,.3f,.4f,1},    // color de lineas
  control_color[]={.5f,.5f,.1f,1}, // poliedros y poligonos de control
  point_color[]={.3f,.3f,.1f,1},   // puntos sueltos o puntos de control
  linewidth=1,pointsize=5, // ancho de lineas y puntos
  escala=100,escala0, // escala de los objetos window/modelo pixeles/unidad
  eye[]={0,0,5}, target[]={0,0,0}, up[]={0,1,0}, // camara, mirando hacia y vertical
  znear=2, zfar=8, //clipping planes cercano y alejado de la camara (en 5 => veo de 3 a -3)
  lpos[]={2,1,5,1}, // posicion luz, l[4]: 0 => direccional -- 1 => posicional
  amy,amy0, // angulo del modelo alrededor del eje y
  ac0,rc0; // angulo resp x y distancia al target de la camara al clickear
  
  //matrices
  MATRIX4X4 lightproyMatrix, lightviewMatrix,
        camproyMatrix, camviewMatrix;
  
  double w0=(double)w/2/escala,h0=(double)h/2/escala; // semiancho y semialto en el target

//tamaño del shadow map
  int shadowMapSize=512;
  
 //texturas
  GLuint shadowMapTexture;

bool // variables de estado de este programa
  luz_camara=true,  // luz fija a la camara o al espacio
  perspectiva=true, // perspectiva u ortogonal
  rota=false,       // gira continuamente los objetos respecto de y
  animado=false,    // el objeto actual es animado
  dibuja=true,      // false si esta minimizado
  wire=false,       // dibuja lineas o no
  relleno=true,     // dibuja relleno o no
  smooth=true,      // normales por nodo o por plano
  cl_info=true,     // informa por la linea de comandos
  antialias=false,  // antialiasing
  color=false,      // color/material
  blend=false;      // transparencias

short modifiers=0;  // ctrl, alt, shift (de GLUT)

static const double R2G=45/atan(1.0);

// temporizador:
static const int ms_lista[]={1,2,5,10,20,50,100,200,500,1000,2000,5000},ms_n=12;
static int ms_i=4,msecs=ms_lista[ms_i]; // milisegundos por frame

//------------------------------------------------------------
// redibuja los objetos
extern void drawObjects(int cambia=0);
// Cada vez que hace un redisplay
void Display_cb() {
//  if (cl_info) cout << "Display\t"; cout.flush();
  if (!dibuja) return;
  
  // Primer paso - desde el punto de vista de la luz
  
  // borra los buffers de pantalla y z
  glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(lightproyMatrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(lightviewMatrix);
  
  //Usamos el viewport del tamaño del shadow map
  glViewport(0,0,shadowMapSize,shadowMapSize);
  
  //descartamos caras delanteras para que las traseras esten en el shadow map
  glCullFace(GL_FRONT);
  
  //no escribimos en el buffer de color, y usamos flat shading para acelerar
  glShadeModel(GL_FLAT);
  glColorMask(0,0,0,0);
    
  drawObjects();
  
  
  // leo el z-buffer
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLint width = viewport[2];
  GLint height = viewport[3];
  GLfloat *pixels = new GLfloat[width * height];
  glPixelStorei(GL_UNPACK_ROW_LENGTH, viewport[2]);
  int rowSkip = 0;
  int pixelSkip = 0;
  glPixelStorei(GL_PACK_SKIP_PIXELS, pixelSkip);
  glPixelStorei(GL_PACK_SKIP_ROWS, rowSkip);
  glReadPixels(0, 0, width, height,GL_DEPTH_COMPONENT,GL_FLOAT, pixels);
  
  //guardamos los pixeles en un archivo separado por comas
//  std::ofstream pixelFile;
//  pixelFile.open("pixels.csv");
//  
//  int numPixels=width*height;
//  int testH = 0;
//  for(int i = 0; i < numPixels; i++) {
//    if((i % (width)) == 0){
//      pixelFile << "\n";
//      testH++;
//    }
//    pixelFile << pixels[i] << ", " ;
//  }
//  pixelFile.close();
  
  
  //Copiamos el buffer al buffer de color
  /*  glColorMask(1,1,1,1);*/
//  glDrawBuffer(GL_FRONT);
//  rowSkip = 2;
//  pixelSkip = 2;
//  glPixelStorei(GL_UNPACK_SKIP_PIXELS, pixelSkip);
//  glPixelStorei(GL_UNPACK_SKIP_ROWS, rowSkip);
//  glDrawPixels( width, height,GL_RGB,GL_FLOAT, pixels);
  
  //leemos el depth buffer y lo ponemos en la textura para el shadow map
  
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glCopyTexSubImage2D(GL_TEXTURE_2D,0,0,0,0,0,shadowMapSize,shadowMapSize);
  
  //restauramos estados cambiados
  glCullFace(GL_BACK);
  glShadeModel(GL_SMOOTH);
  glColorMask(1,1,1,1);
  
  //Segundo paso - Dibujamos desde el punto de vista de la camara con luz poco brillante
  
  glClear(GL_DEPTH_BUFFER_BIT);
  
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(camproyMatrix);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(camviewMatrix);
  
//  glViewport(0,0,w,h);  //se puede sacar?
  
  // Seteamos la luz para las zonas con sombras.
  
  glLightfv(GL_LIGHT0,GL_AMBIENT,white*0.2f);
  glLightfv(GL_LIGHT0,GL_DIFFUSE,white*0.2f);
  glLightfv(GL_LIGHT0,GL_SPECULAR,black);
  // caras de atras y adelante distintos (1) o iguales (0)
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER,0);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHTING);
    
  drawObjects();
//    
//    // Tercer paso - Dibujamos con una luz brillante
//    
//    glLightfv(GL_LIGHT0,GL_DIFFUSE,white);
//    glLightfv(GL_LIGHT0,GL_SPECULAR,white);
//    
  //definimos la matriz de sesgo (para mapear los valores entre -1 y 1 a valores entre 0 y 1
  static MATRIX4X4 biasMatrix (0.5f, 0.0f, 0.0f, 0.0f, 
                               0.0f, 0.5f, 0.0f, 0.0f,
                               0.0f, 0.0f, 0.5f, 0.0f,
                               0.5f, 0.5f, 0.5f, 1.0f);

  //calculamos la matriz de proyeccion, que nos lleva desde coordenadas del ojo
  //a coordeandas de la proyeccion de la luz (projective texture mapping) 
  MATRIX4X4 textureMatrix=biasMatrix*lightproyMatrix*lightviewMatrix;
    
  // seteamos las coordenadas de generacion de la textura
    
  glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_S, GL_EYE_PLANE, textureMatrix.GetRow(0));
  glEnable(GL_TEXTURE_GEN_S);
    
  glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_T, GL_EYE_PLANE, textureMatrix.GetRow(1));
  glEnable(GL_TEXTURE_GEN_T);
    
  glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_R, GL_EYE_PLANE, textureMatrix.GetRow(2));
  glEnable(GL_TEXTURE_GEN_R);
    
  glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
  glTexGenfv(GL_Q, GL_EYE_PLANE, textureMatrix.GetRow(3));
  glEnable(GL_TEXTURE_GEN_Q);
   
  // ahora enlazamos y habilitamos la textura del shadow map
  glBindTexture(GL_TEXTURE_2D,shadowMapTexture);
  glEnable(GL_TEXTURE_2D);
    
  glutSwapBuffers();

#ifdef _DEBUG
  // chequea errores
  int errornum=glGetError();
  while(errornum!=GL_NO_ERROR){
    if (cl_info){
           if(errornum==GL_INVALID_ENUM)
             cout << "GL_INVALID_ENUM" << endl;
      else if(errornum==GL_INVALID_VALUE)
             cout << "GL_INVALID_VALUE" << endl;
      else if (errornum==GL_INVALID_OPERATION)
             cout << "GL_INVALID_OPERATION" << endl;
      else if (errornum==GL_STACK_OVERFLOW)
             cout << "GL_STACK_OVERFLOW" << endl;
      else if (errornum==GL_STACK_UNDERFLOW)
             cout << "GL_STACK_UNDERFLOW" << endl;
      else if (errornum==GL_OUT_OF_MEMORY)
             cout << "GL_OUT_OF_MEMORY" << endl;
    }
    errornum=glGetError();
  }
#endif // _DEBUG
}

//------------------------------------------------------------
// Regenera la matriz de proyeccion
// cuando cambia algun parametro de la vista
void regen() {
//  if (cl_info) cout << "regen" << endl;
  if (!dibuja) return;

  // 
  glMatrixMode(GL_PROJECTION);  glLoadIdentity();

  

  // frustum, perspective y ortho son respecto al eye pero con los z positivos (delante del ojo)
  if (perspectiva){ // perspectiva
    double // "medio" al cuete porque aqui la distancia es siempre 5
      delta[3]={(target[0]-eye[0]), // vector ojo-target
                (target[1]-eye[1]),
                (target[2]-eye[2])},
      dist=sqrt(delta[0]*delta[0]+
                delta[1]*delta[1]+
                delta[2]*delta[2]);
    w0*=znear/dist,h0*=znear/dist;  // w0 y h0 en el near
    glFrustum(-w0,w0,-h0,h0,znear,zfar);
    
    
  }
  else { // proyeccion ortogonal
    glOrtho(-w0,w0,-h0,h0,znear,zfar);
  }

  glMatrixMode(GL_MODELVIEW); glLoadIdentity(); // matriz del modelo->view

  if (luz_camara) // luz fija a la camara
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz
  gluLookAt(   eye[0],   eye[1],   eye[2],
            target[0],target[1],target[2],
                up[0],    up[1],    up[2]);// ubica la camara

  glRotatef(amy,0,1,0); // rota los objetos alrededor de y

  if (!luz_camara) // luz fija en el espacio del modelo
    glLightfv(GL_LIGHT0,GL_POSITION,lpos);  // ubica la luz

//  glutPostRedisplay();
  Display_cb();
}

//------------------------------------------------------------
// Animacion

// Si no hace nada hace esto
// glutIdleFunc lo hace a la velocidad que de la maquina
// glutTimerFunc lo hace cada tantos milisegundos

// glutTimerFunc funciona mal, hace cosas raras que son muy visibles
// cuando la espera (msecs) es grande

// El "framerate" real (cuadros por segundo)
// depende de la complejidad del modelo (lod) y la aceleracion por hardware
void Idle_cb() {
  static int suma,counter=0;// esto es para analisis del framerate real
  // Cuenta el lapso de tiempo
  // (int!!?? en seguida se hace >MAX_INT pero ... funciona)
  static int anterior=glutGet(GLUT_ELAPSED_TIME); // milisegundos desde que arranco
  if (msecs!=1){ // si msecs es 1 no pierdo tiempo
    int tiempo=glutGet(GLUT_ELAPSED_TIME), lapso=tiempo-anterior;
    if (lapso<msecs) return;
    suma+=lapso;
    if (++counter==100) {
      cout << "<ms/frame>= " << suma/100.0 << endl;
      counter=suma=0;
    }
    anterior=tiempo;
  }
  if (rota) { // los objetos giran 1 grado alrededor de y
    amy+=1; if (amy>360) amy-=360;
    regen();
  }
  else Display_cb(); // redibuja
}

//------------------------------------------------------------
// Maneja cambios de ancho y alto de la ventana
void Reshape_cb(int width, int height){
  h=height; w=width;
  if (cl_info) cout << "reshape: " << w << "x" << h << endl;

  if (w==0||h==0) {// minimiza
    dibuja=false; // no dibuja mas
    glutIdleFunc(0); // no llama a cada rato a esa funcion
    return;
  }
  else if (!dibuja&&w&&h){// des-minimiza
    dibuja=true; // ahora si dibuja
    glutIdleFunc(Idle_cb); // registra de nuevo el callback
  }

  glViewport(0,0,w,h); // region donde se dibuja

 regen(); //regenera la matriz de proyeccion
}

//------------------------------------------------------------
// Teclado
/*
GLUT ACTIVE SHIFT //Set if the Shift modifier or Caps Lock is active.
GLUT ACTIVE CTRL //Set if the Ctrl modifier is active.
GLUT ACTIVE ALT //Set if the Alt modifier is active.
*/
inline short get_modifiers() {return modifiers=(short)glutGetModifiers();}

// Maneja pulsaciones del teclado (ASCII keys)
// x,y posicion del mouse cuando se teclea
void Keyboard_cb(unsigned char key,int x=0,int y=0) {
  switch (key){
    case 'a': case 'A': // Antialiasing
      antialias=!antialias;
      if (antialias){
        glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Antialiasing" << endl;
      }
      else {
        glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
        if (cl_info) cout << "Sin Antialiasing" << endl;
      }
      break;
    case 'c': case 'C': // cambia
      drawObjects((key=='c') ? 1 : -1);
      break;
    case 'f': case 'F': // relleno
      relleno=!relleno;
      if (cl_info) cout << ((relleno)? "Relleno" : "Sin Relleno") << endl;
      break;
    case 'i': case 'I': // info
      cl_info=!cl_info;
      cout << ((cl_info)? "Info" : "Sin Info") << endl;
      break;
    case 'j': case 'J': // luz fija a la camara o en el espacio
      luz_camara=!luz_camara;
      if (cl_info)
        cout << "Luz fija " << ((luz_camara)? "a la camara" : "en el espacio") << endl;
      regen();
      break;
    case 'l': case 'L': // wire
      wire=!wire;
      if (cl_info) cout << ((wire)? "Lineas" : "Sin Lineas") << endl;
      break;
    case 'm': case 'M': // color/material
      color=!color;
      if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);
      if (cl_info) cout << ((color)? "Color" : "Material") << endl;
      break;
    case 'p': case 'P':  // perspectiva
      perspectiva=!perspectiva;
      if (cl_info) cout << ((perspectiva)? "Perspectiva" : "Ortogonal") << endl;
      regen();
      break;
    case 'r': case 'R': // rotacion
      rota=!rota;
      if (cl_info) cout << ((rota)? "Gira" : "No Gira") << endl;
      break;
    case 's': case 'S': // smooth
      smooth=!smooth;
      glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);
      if (cl_info) cout << ((smooth)? "Suave" : "Facetado") << endl;
      break;
    case 't': case 'T': // transparencia
      blend=!blend;
      if (blend){
        glEnable(GL_BLEND);
        if (cl_info) cout << "Transparencia" << endl;
      }
      else {
        glDisable(GL_BLEND);
        if (cl_info) cout << "Sin Transparencia" << endl;
      }
      break;
    case 27: // escape => exit
      get_modifiers();
      if (!modifiers)
        exit(EXIT_SUCCESS);
      break;
    case '+': case '-': // lod
      if (key=='+') lod++; else {lod--; if (lod==0) lod=1;}
      if (cl_info) cout << "Nivel de detalle: "  << lod << endl;
      break;
  }
  if (!(animado|rota)) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback
  Display_cb();
}

// Special keys (non-ASCII)
/*
  GLUT KEY F[1,12] F[1,12] function key.
  GLUT KEY LEFT Left directional key.
  GLUT KEY UP Up directional key.
  GLUT KEY RIGHT Right directional key.
  GLUT KEY DOWN Down directional key.
  GLUT KEY PAGE UP Page up directional key.
  GLUT KEY PAGE DOWN Page down directional key.
  GLUT KEY HOME Home directional key.
  GLUT KEY END End directional key.
  GLUT KEY INSERT Inset directional key.
*/
// aca es int key
void Special_cb(int key,int xm=0,int ym=0) {
  if (key==GLUT_KEY_F4){ // alt+f4 => exit
    get_modifiers();
    if (modifiers==GLUT_ACTIVE_ALT)
      exit(EXIT_SUCCESS);
  }
  if (key==GLUT_KEY_UP||key==GLUT_KEY_DOWN){ // camara
    // la camara gira alrededor del eje -x
    double yc=eye[1]-target[1],zc=eye[2]-target[2],
           rc=sqrt(yc*yc+zc*zc),ac=atan2(yc,zc);
    ac+=((key==GLUT_KEY_UP) ? 1 : -1)/R2G;
    yc=rc*sin(ac); zc=rc*cos(ac);
    up[1]=zc; up[2]=-yc;  // perpendicular
    eye[1]=target[1]+yc; eye[2]=target[2]+zc;
    regen();
  }
  if (key==GLUT_KEY_LEFT){ // gira
    amy-=1;
    regen();
  }
  if (key==GLUT_KEY_RIGHT){ // gira
    amy+=1;
    regen();
  }
  if (key==GLUT_KEY_PAGE_UP||key==GLUT_KEY_PAGE_DOWN){ // velocidad
    if (key==GLUT_KEY_PAGE_DOWN) ms_i++;
    else ms_i--;
    if (ms_i<0) ms_i=0; if (ms_i==ms_n) ms_i--;
    msecs=ms_lista[ms_i];
    if (cl_info){
      if (msecs<1000)
        cout << 1000/msecs << "fps" << endl;
      else
        cout << msecs/1000 << "s/frame)" << endl;
    }
  }
}

//------------------------------------------------------------
// Menu
void Menu_cb(int value)
{
  switch (value){
    case 'a':
      Keyboard_cb('a');
      return;
    case 'c':
      Keyboard_cb('c');
      return;
    case 'C':
      Keyboard_cb('C');
      return;
    case 'f':
      Keyboard_cb('f');
      return;
    case 'i':
      Keyboard_cb('i');
      return;
    case 'j':
      Keyboard_cb('j');
      return;
    case 'k':
      Keyboard_cb('k');
      return;
    case 'l':
      Keyboard_cb('l');
      return;
    case 'm':
      Keyboard_cb('m');
      return;
    case 'p':
      Keyboard_cb('p');
      return;
    case 'r':
      Keyboard_cb('r');
      return;
    case 's':
      Keyboard_cb('s');
      return;
    case 't':
      Keyboard_cb('t');
      return;
    case '+':
      Keyboard_cb('+');
      return;
    case '-':
      Keyboard_cb('-');
      return;
    case (256+GLUT_KEY_UP):
      Special_cb(GLUT_KEY_UP);
      return;
    case (256+GLUT_KEY_DOWN):
      Special_cb(GLUT_KEY_DOWN);
      return;
    case (256+GLUT_KEY_LEFT):
      Special_cb(GLUT_KEY_LEFT);
      return;
    case (256+GLUT_KEY_RIGHT):
      Special_cb(GLUT_KEY_RIGHT);
      return;
    case (256+GLUT_KEY_PAGE_UP):
      Special_cb(GLUT_KEY_PAGE_UP);
      return;
    case (256+GLUT_KEY_PAGE_DOWN):
      Special_cb(GLUT_KEY_PAGE_DOWN);
      return;
    case 27: //esc
      exit(EXIT_SUCCESS);
  }
}

//------------------------------------------------------------
// Movimientos del mouse
void Motion_cb(int xm, int ym){ // drag
  if (boton==GLUT_LEFT_BUTTON){
    if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
      escala=escala0*exp((yclick-ym)/100.0);
      regen();
    }
    else { // manipulacion
      double yc=eye[1]-target[1],zc=eye[2]-target[2];
      double ac=ac0+(ym-yclick)*180.0/h/R2G;
      yc=rc0*sin(ac); zc=rc0*cos(ac);
      up[1]=zc; up[2]=-yc;  // perpendicular
      eye[1]=target[1]+yc; eye[2]=target[2]+zc;
      amy=amy0+(xm-xclick)*180.0/w;
      regen();
    }
  }
}

// Clicks del mouse
// GLUT LEFT BUTTON, GLUT MIDDLE BUTTON, or GLUT RIGHT BUTTON
// The state parameter is either GLUT UP or GLUT DOWN
// glutGetModifiers may be called to determine the state of modifier keys
void Mouse_cb(int button, int state, int x, int y){
  static bool old_rota=false;
  if (button==GLUT_LEFT_BUTTON){
    if (state==GLUT_DOWN) {
      xclick=x; yclick=y;
      boton=button;
      old_rota=rota; rota=false;
      get_modifiers();
      glutMotionFunc(Motion_cb); // callback para los drags
      if (modifiers==GLUT_ACTIVE_SHIFT){ // cambio de escala
        escala0=escala;
      }
      else { // manipulacion
        double yc=eye[1]-target[1],zc=eye[2]-target[2];
        rc0=sqrt(yc*yc+zc*zc); ac0=atan2(yc,zc);
        amy0=amy;
      }
    }
    else if (state==GLUT_UP){
      rota=old_rota;
      boton=-1;
      glutMotionFunc(0); // anula el callback para los drags
    }
  }
}

//------------------------------------------------------------
// pregunta a OpenGL por el valor de una variable de estado
int integerv(GLenum pname){
  int value;
  glGetIntegerv(pname,&value);
  return value;
}
#define _PRINT_INT_VALUE(pname) #pname << ": " << integerv(pname) <<endl

//------------------------------------------------------------
// Inicializa GLUT y OpenGL
void initialize() {
  // pide z-buffer, color RGBA y double buffering
  glutInitDisplayMode(GLUT_DEPTH|GLUT_RGBA|GLUT_DOUBLE);

  glutInitWindowSize(w,h); glutInitWindowPosition(50,50);

  glutCreateWindow("Shadow Mapping"); // crea el main window

  //declara los callbacks
  //los que no se usan no se declaran
  glutDisplayFunc(Display_cb); // redisplays
  glutReshapeFunc(Reshape_cb); // cambio de alto y ancho
  glutKeyboardFunc(Keyboard_cb); // teclado
  glutSpecialFunc(Special_cb); // teclas especiales
  glutMouseFunc(Mouse_cb); // botones picados
  if (!(dibuja&&(animado|rota))) glutIdleFunc(0); // no llama a cada rato a esa funcion
  else glutIdleFunc(Idle_cb); // registra el callback

  // crea el menu
  glutCreateMenu(Menu_cb);
    glutAddMenuEntry("     [p]_Perspectiva/Ortogonal ", 'p');
    glutAddMenuEntry("     [c]_Objeto Siguiente      ", 'c');
    glutAddMenuEntry("     [C]_Objeto Anterior       ", 'C');
    glutAddMenuEntry("     [r]_Rota                  ", 'r');
    glutAddMenuEntry("     [+]_+Nivel de Detalle     ", '+');
    glutAddMenuEntry("     [-]_-Nivel de Detalle     ", '-');
    glutAddMenuEntry("     [l]_Lineas                ", 'l');
    glutAddMenuEntry("     [f]_Caras Rellenas        ", 'f');
    glutAddMenuEntry("     [m]_Color/Material        ", 's');
    glutAddMenuEntry("     [s]_Suave/Facetado        ", 's');
    glutAddMenuEntry("     [a]_Antialiasing          ", 'a');
    glutAddMenuEntry("     [t]_Transparencia         ", 't');
    glutAddMenuEntry("    [Up]_Sube Camara           ", (256+GLUT_KEY_UP));
    glutAddMenuEntry("  [Down]_Baja Camara           ", (256+GLUT_KEY_DOWN));
    glutAddMenuEntry("  [Left]_Gira objeto           ", (256+GLUT_KEY_LEFT));
    glutAddMenuEntry(" [Right]_Gira objeto           ", (256+GLUT_KEY_RIGHT));
    glutAddMenuEntry("  [PgUp]_Aumenta Framerate     ", (256+GLUT_KEY_PAGE_UP));
    glutAddMenuEntry("  [Pgdn]_Disminuye Framerate   ", (256+GLUT_KEY_PAGE_DOWN));
    glutAddMenuEntry("     [j]_Luz fija ON/OFF       ", 'j');
    glutAddMenuEntry("     [i]_Info ON/OFF           ", 'i');
    glutAddMenuEntry("   [Esc]_Exit                  ", 27);
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  // ========================
  // estado normal del OpenGL
  // ========================
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LEQUAL); // habilita el z-buffer
  glEnable(GL_NORMALIZE); // para que el scaling no moleste
  
  glEnable(GL_POLYGON_OFFSET_FILL); glPolygonOffset (1,1); // coplanaridad

  // interpola normales por nodos o una normal por plano
  glShadeModel((smooth) ? GL_SMOOTH : GL_FLAT);

  // ancho de lineas y puntos
  glLineWidth(linewidth); glPointSize(pointsize);

  // antialiasing
  if (antialias){
    glEnable(GL_POINT_SMOOTH); glEnable(GL_LINE_SMOOTH); glEnable(GL_POLYGON_SMOOTH);
  }
  else {
    glDisable(GL_POINT_SMOOTH); glDisable(GL_LINE_SMOOTH); glDisable(GL_POLYGON_SMOOTH);
  }
  glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
  glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
  
  
  //agregamos nosotros
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

//  // transparencias
//  if (blend) glEnable(GL_BLEND);
//  else glDisable(GL_BLEND);
//  glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

  // color de fondo
  glClearColor(fondo[0],fondo[1],fondo[2],0);

  // direccion de los poligonos
  glFrontFace(GL_CCW); glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  glCullFace(GL_BACK); // se habilita por objeto (cerrado o abierto)

  //creamos la textura del shadow map
  glGenTextures(1, &shadowMapTexture);
  glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize,0,
    GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
  
  
  // color o material+luces
  if (color) glDisable(GL_LIGHTING); else glEnable(GL_LIGHTING);

  // material estandar
  
  glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);
  
  static const float specular[]={1.0f,1.0f,1.0f,1.0f};
  static const int shininess=16; 
  
  glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
  glMateriali(GL_FRONT,GL_SHININESS,shininess);
    
  
  // Ahora empieza la magia. Seteamos las matrices para la camara y la luz
  // y las guardamos en variables globales
  glPushMatrix();
  
  glLoadIdentity();
  glFrustum(-w0,w0,-h0,h0,znear,zfar);  //generamos la matriz de proyeccion de la camara
  glGetFloatv(GL_MODELVIEW_MATRIX,camproyMatrix); // la guardamos en camproyMatrix
  
  glLoadIdentity();
  gluLookAt(eye[0],eye[1],eye[2],
            target[0],target[1],target[2],
            up[0],up[1],up[2]); // ubica la camara
  glGetFloatv(GL_MODELVIEW_MATRIX,camviewMatrix); // la guardamos en camviewMatrix  
  
  glLoadIdentity();
  
  double delta[3]={(target[0]-lpos[0]), // vector luz-target
            (target[1]-lpos[1]),
            (target[2]-lpos[2])},
         dist=sqrt(delta[0]*delta[0]+
                   delta[1]*delta[1]+
                   delta[2]*delta[2]);
  w0*=znear/dist,h0*=znear/dist;  // w0 y h0 en el near
  
  glFrustum(-w0,w0,-h0,h0,znear,zfar);  //generamos la matriz de proyeccion de la luz
  glGetFloatv(GL_MODELVIEW_MATRIX,lightproyMatrix);
  
  glLoadIdentity();
  gluLookAt(lpos[0],lpos[1],lpos[2],
            target[0],target[1],target[2],
            up[0],up[1],up[2]); //ubicacion de la camara en posicion de la luz_camara
  glGetFloatv(GL_MODELVIEW_MATRIX,lightviewMatrix);
  
  glPopMatrix();
  


  

  // ========================
  // info
  if (cl_info)
    cout << "Vendor:         " << glGetString(GL_VENDOR) << endl
         << "Renderer:       " << glGetString(GL_RENDERER) << endl
         << "GL_Version:     " << glGetString(GL_VERSION) << endl
         << "GL_Extensions:  " << glGetString(GL_EXTENSIONS) << endl
         << "GLU_Version:    " << gluGetString(GLU_VERSION) << endl
         << "GLU_Extensions: " << gluGetString(GLU_EXTENSIONS) << endl
         << _PRINT_INT_VALUE(GL_DOUBLEBUFFER)
         << _PRINT_INT_VALUE(GL_STEREO)
         << _PRINT_INT_VALUE(GL_AUX_BUFFERS)
         << _PRINT_INT_VALUE(GL_RED_BITS)
         << _PRINT_INT_VALUE(GL_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ALPHA_BITS)
         << _PRINT_INT_VALUE(GL_DEPTH_BITS)
         << _PRINT_INT_VALUE(GL_STENCIL_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_RED_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_GREEN_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_BLUE_BITS)
         << _PRINT_INT_VALUE(GL_ACCUM_ALPHA_BITS)
         ;
  // ========================

  regen(); // para que setee las matrices antes del 1er draw
}

//------------------------------------------------------------
// main
int main(int argc,char** argv) {
  glutInit(&argc,argv);// inicializa glut
  initialize(); // condiciones iniciales de la ventana y OpenGL
  glutMainLoop(); // entra en loop de reconocimiento de eventos
  return 0;
}
