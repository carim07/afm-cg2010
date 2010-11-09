#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include "Malla.h"
using namespace std;

Malla *malla;

void Malla::Load(const char *fname) {
  e.clear(); p.clear();
  ifstream f(fname);
  int i,nv;
  f>>nv;
  float x,y,z;
  for (i=0;i<nv;i++) {
    f>>x>>y>>z;
    p.push_back(Nodo(x,y,z));
  }
  int ne;
  f>>ne;
  int v0,v1,v2,v3;
  for (i=0;i<ne;i++) {
    f>>nv>>v0>>v1>>v2;
    if (nv==3) { AgregarElemento(v0,v1,v2); }
    else { f>>v3; AgregarElemento(v0,v1,v2,v3); }
  }
  f.close();
  MakeVecinos();
  MakeNormales();
}

Malla::Malla(const char *fname) {
  if (fname) Load(fname);
}

void Malla::Save(const char *fname) {
  ofstream f(fname);
  f<<p.size()<<endl;
  unsigned int i,j;
  for (i=0;i<p.size();i++)
    f<<p[i].x[0]<<' '<<p[i].x[1]<<' '<<p[i].x[2]<<endl;
  f<<e.size()<<endl;
  for (i=0;i<e.size();i++) {
    f<<e[i].nv;
    for (j=0;j<e[i].nv;j++)
      f<<' '<<e[i][j];
    f<<endl;
  }
  f.close();
}

void Malla::AgregarElemento(int n0, int n1, int n2, int n3) {
  int ie=e.size(); e.push_back(Elemento(n0,n1,n2,n3));
  p[n0].e.push_back(ie); p[n1].e.push_back(ie);
  p[n2].e.push_back(ie); if (n3>=0) p[n3].e.push_back(ie);
  
}

void Malla::ReemplazarElemento(int ie, int n0, int n1, int n2, int n3) {
  Elemento &ei=e[ie];
  for (unsigned int i=0;i<ei.nv;i++) {
    vector<int> &ve=p[ei[i]].e;
    ve.erase(find(ve.begin(),ve.end(),ie));
  }
  ei.SetNodos(n0,n1,n2,n3);
  p[n0].e.push_back(ie); p[n1].e.push_back(ie); p[n2].e.push_back(ie); 
  if (n3>=0) p[n3].e.push_back(ie); 
}

void Malla::MakeNormales() {
  normal.resize(p.size());
  for (unsigned int i=0;i<p.size();i++) {
    vector<int> &en=p[i].e;
    Nodo n; int k;
    for (unsigned int j=0;j<en.size();j++) {
      Elemento &ej=e[en[j]];
      k=ej.Indice(i);
      n+=(p[ej[k]]-p[ej[k-1]])%(p[ej[k+1]]-p[ej[k]]);
    }
    if (en.size()) normal[i]=n/en.size();
  }
}

void Malla::Draw(bool relleno) {
  // dibuja los Elementos
  unsigned int i;
  if (relleno) {
    glEnable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
  } else {
    glDisable(GL_LIGHTING);
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
  }
  for (i=0;i<e.size();i++) {
    if (e[i].nv==4) {
      glBegin(GL_QUADS);
      glNormal3fv(normal[e[i][0]].x); glVertex3fv(p[e[i][0]].x); 
      glNormal3fv(normal[e[i][1]].x); glVertex3fv(p[e[i][1]].x); 
      glNormal3fv(normal[e[i][2]].x); glVertex3fv(p[e[i][2]].x); 
      glNormal3fv(normal[e[i][3]].x); glVertex3fv(p[e[i][3]].x); 
    } else {
      glBegin(GL_TRIANGLES);
      glNormal3fv(normal[e[i][0]].x); glVertex3fv(p[e[i][0]].x);
      glNormal3fv(normal[e[i][1]].x); glVertex3fv(p[e[i][1]].x);
      glNormal3fv(normal[e[i][2]].x); glVertex3fv(p[e[i][2]].x);
    }
    glEnd();
  }
  // dibuja solo los nodos sueltos
  glDisable(GL_LIGHTING);
  glBegin(GL_POINTS);
  for (i=0;i<p.size();i++) {
    glVertex3fv(p[i].x);
    if (p[i].e.size()==0) glVertex3fv(p[i].x);
  }
  glEnd();
}

void Malla::MakeVecinos() {
  unsigned int i,j,k;
  int ix;
  for (i=0;i<p.size();i++) p[i].es_frontera=false;
  for (i=0;i<e.size();i++) e[i].v[0]=e[i].v[1]=e[i].v[2]=e[i].v[3]=-1;
  for (i=0;i<e.size();i++) { // por cada elemento
    for (j=0;j<e[i].nv;j++) { // por cada arista
      if (e[i].v[j]>=0) continue; // ya se hizo
      int in0=e[i][j]; // 1er nodo de la arista
      int in1=e[i][j+1]; // 2do nodo de la arista
      for (k=0;k<p[in0].e.size();k++) { // recorro los Elementos del primer nodo
        if (p[in0].e[k]==int(i)) continue; // es este mismo
        ix=e[p[in0].e[k]].Indice(in1);
        if (ix<0) continue; 
        // tiene al 2do
        e[i].v[j]=p[in0].e[k]; // ese es el vecino
        e[p[in0].e[k]].v[ix]=i;
        break;
      }
      if (k==p[in0].e.size())
        p[e[i][j]].es_frontera=p[e[i][j+1]].es_frontera=true;
    }
  }
}

//// da los nodos "vecinos" de un nodo (otros nodos que comparten con este una aristas)
//const vector<int> &Malla::NodosVecinos(unsigned int in) {
//  static vector<int> vd; vd.clear();
//  vector<int> &eni=p[in].e; // elementos del nodo i
//  for (unsigned int i=0;i<eni.size();i++) {
//    int ie=eni[i]; Elemento &q=e[ie];
//    int ii=q.Indice(in);
//    if (q.v[ii]<ie) vd.push_back(q[ii+1]);
//    if (q.v[(ii+q.nv-1)%q.nv]<ie) vd.push_back(q[(ii+q.nv-1)%q.nv]); // por si hay fronteras
//  }
//  return vd;
//}

void Malla::Subdivide() {
  int i,j, tamanio=p.size();  //tamanio del vector de nodos
  for (i=0;i<e.size();i++) {
    
    Nodo ntemp;
    for (j=0;j<e[i].nv;j++){
      ntemp+=p[e[i][j]];
    }
    ntemp/=e[i].nv;
    p.push_back(ntemp);
  }
  Mapa m;
  

  for (i=0;i<e.size();i++){
    int nv=e[i].nv;
    for (j=0;j<nv;j++){
      int n1=e[i][j], n2=e[i][j+1]; // nodo j y nodo j+1 del elemento i (notar la sobrecarga de la suma modulo nv en el []
      if (!(m.find(Arista(n1,n2))!=m.end())){  // esto es para no contar dos veces la misma arista ;)
      
      int pc1=i+tamanio, pc2=e[i].v[j]+tamanio;  // centroides del elemento i y del vecino j del elemento i (se suma tamanio porque se guardan ordenados los nodos centroides
      
      Nodo nuevo=(p[n1]+p[n2]+p[pc1]+p[pc2])/4; //nuevo nodo promedio de los nodos de la arista y los centroides de los elementos que comparten la arista
      
      m[Arista(n1,n2)]=p.size(); // guardamos la arista entre n1 y n2, y la posicion del punto que agregamos. funciona porque esta sobrecargado en modulo nv
      p.push_back(nuevo);
      }
    }
  }
  
  vector<Elemento> etemp;
  
  for (i=0;i<e.size();i++){
    int nv=e[i].nv;
   
    Elemento nuevo1(m[Arista(e[i].n[0],e[i].n[1])],e[i].n[1],m[Arista(e[i].n[1],e[i].n[2])],i+tamanio); //al elemento lo conforman el nodo de la arista, el nodo original 1 del elemento, el nodo de la otra arista, el centroide
    Elemento nuevo2(i+tamanio,m[Arista(e[i].n[1],e[i].n[2])],e[i].n[2],m[Arista(e[i].n[2],e[i].n[3])]); //para entender, dibujar el cuadrado y ver q voy definiendo los elementos empezando abajo-izquierda
    Elemento nuevo3(m[Arista(e[i].n[3],e[i].n[0])],i+tamanio,m[Arista(e[i].n[2],e[i].n[3])],e[i].n[3]); //siguiendo abajo-derecha, arriba-derecha, arriba-izquierda
    Elemento nuevo0(e[i].n[0],m[Arista(e[i].n[0],e[i].n[1])],i+tamanio,m[Arista(e[i].n[3],e[i].n[0])]); // reemplazo el elemento actual por el elemento inferior izquierdo

    etemp.push_back(nuevo0);
    etemp.push_back(nuevo1);
    etemp.push_back(nuevo2);
    etemp.push_back(nuevo3);
    
  }
  e=etemp;
  
  //falta mover los nodos originales
    
  
  
  // @@@@@: Implementar Catmull-Clark... lineamientos:
  //  0) Los nodos originales van de 0 a Nn-1, los elementos orignales de 0 a Ne-1
  //  1) Por cada elemento, agregar el centroide (nuevos nodos: Nn a Nn+Ne-1)
  //  2) Por cada arista de cada cara, agregar un pto en el medio que es
  //      promedio de los vertices de la arista y los centroides de las caras 
  //      adyacentes. Aca hay que usar los elementos vecinos.
  //      En los bordes, cuando no hay vecinos, es simplemente el promedio de los 
  //      vertices de la arista 
  //      Hay que evitar procesar dos veces la misma arista (como?)
  //      Mas adelante vamos a necesitar encontrar los puntos que agregamos para 
  //      cada arista, y ya que no se puede relacionar el indice de los nodos nuevos
  //      (la cantidad de aristas por nodo es variable), se sugiere usar Mapa como 
  //      estructura auxiliar
  //  3) Armar los elementos nuevos
  //      Los quads se dividen en 4, (uno reemplaza al original, los otros 3 se agregan)
  //      Los triangulos se dividen en 3, (uno reemplaza al original, los otros 2 se agregan)
  //      Para encontrar los nodos de las aristas usar el mapa que armaron en el paso 2
  //  4) Calcular las nuevas posiciones de los nodos originales
  //      Para nodos interiores: (4r-f+(n-3)p)/n
  //         f=promedio de nodos interiores de las caras (los agregados en el paso 1)
  //         r=promedio de los pts medios de las aristas (los agregados en el paso 2)
  //         p=posicion del nodo original
  //         n=cantidad de elementos para ese nodo
  //      Para nodos del borde: (r+p)/2
  //         r=promedio de los dos pts medios de las aristas
  //         p=posicion del nodo original
  //      Ojo: en el paso 3 cambio toda la malla, analizar donde quedan en los nuevos 
  //      elementos (¿de que tipo son?) los nodos de las caras y los de las aristas 
  //      que se agregaron antes.
  // tips:
  //   no es necesario cambiar ni agregar nada fuera de este método, (con Mapa como 
  //     estructura auxiliar alcanza)
  //   sugerencia: probar primero usando el cubo (es cerrado y solo tiene quads)
  //               despues usando la piramide (tambien cerrada, y solo triangulos)
  //               despues el ejemplo plano (para ver que pasa en los bordes)
  //               finalmente el mono (tiene mezcla y elementos sin vecinos)
  //   repaso de como usar un mapa:
  //     para asociar un indice (i) de nodo a una arista (n1-n2): elmapa[Arista(n1,n2)]=i;
  //     para saber si hay un indice asociado a una arista:  ¿elmapa.find(Arista(n1,n2))!=elmapa.end()?
  //     para recuperar el indice (en j) asociado a una arista: int j=elmapa[Arista(n1,n2)];
  
}
