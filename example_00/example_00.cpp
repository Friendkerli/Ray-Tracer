// CS184 Simple OpenGL Example
#include <vector>
#include <iostream>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>

#ifdef _WIN32
static DWORD lastTime;
#else
static struct timeval lastTime;
#endif

#define PI 3.14159265
inline float sqr(float x) { return x*x; }
const int maxtoken = 20;
const int maxchar = 512;
const char* const DELIMITER = " ";
using namespace std;

//****************************************************
// Some Classes
//****************************************************
class Viewport {
  public:
    int w, h; // width and height
};

struct point {
  float x;
  float y;
  float z;
}; 
struct curve
{
  point p1;
  point p2;
  point p3;
  point p4;
};
struct patch
{
  curve c[4];
};

struct curvere
{
  point p;
  point dev;
};

struct patchre
{
  point p;
  point nor;
};

//****************************************************
// Global Variables
//****************************************************
Viewport    viewport;
float SUB;     // subdivision parameter
bool ADP;      // adaptive or not
int numofpatch;
patch *PA;
GLfloat viewangle = 0, tippangle = 0;
int wireframe = 1;
int shade = 1;
GLfloat tx = 0, ty = 0, tz = -10;




void drawLine(point p1, point p2){       
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 1.0f);    
    glVertex3f(p1.x, p1.y, p1.z);    
    glVertex3f(p2.x, p2.y, p2.z);    
    glEnd();
  }

point pointadd(point p1, point p2){
  point p3;
  p3.x = p1.x + p2.x;
  p3.y = p1.y + p2.y;
  p3.z = p1.z + p2.z;
  return p3;
}

point pointsub(point p1, point p2){
  point p3;
  p3.x = p1.x - p2.x;
  p3.y = p1.y - p2.y;
  p3.z = p1.z - p2.z;
  return p3;
}

point sclar(point p, float s){
  point r;
  r.x = p.x * s;
  r.y = p.y * s;
  r.z = p.z * s;
  return r;
}

point cross(point p1, point p2){
  point r;
  r.x = p1.y*p2.z - p1.z*p2.y;
  r.y = p1.z*p2.x - p1.x*p2.z;
  r.z = p1.x*p2.y - p1.y*p2.x;
  return r;

}

curvere bezcurve(curve c, float u){
  point A, B, C, D, E, p1, p2;
  float ru = 1.0 - u;
  A = pointadd(sclar(c.p1, ru), sclar(c.p2, u));
  B = pointadd(sclar(c.p2, ru), sclar(c.p3, u));
  C = pointadd(sclar(c.p3, ru), sclar(c.p4, u));

  D = pointadd(sclar(A, ru), sclar(B, u));
  E = pointadd(sclar(B, ru), sclar(C, u));

  p1 = pointadd(sclar(D, ru), sclar(E, u));
  p2 = sclar(pointsub(E, D), 3.0);

  curvere re;
  re.p = p1;
  re.dev = p2;

  return re;
}

patchre bezpatch(patch pat, float u, float v){
  curve vc;
  vc.p1 = bezcurve(pat.c[0], u).p;
  vc.p2 = bezcurve(pat.c[1], u).p;
  vc.p3 = bezcurve(pat.c[2], u).p;
  vc.p4 = bezcurve(pat.c[3], u).p;

  curve u1, u2, u3, u4;
  u1.p1 = pat.c[0].p1;
  u1.p2 = pat.c[1].p1;
  u1.p3 = pat.c[2].p1;
  u1.p4 = pat.c[3].p1;

  u2.p1 = pat.c[0].p2;
  u2.p2 = pat.c[1].p2;
  u2.p3 = pat.c[2].p2;
  u2.p4 = pat.c[3].p2;

  u3.p1 = pat.c[0].p3;
  u3.p2 = pat.c[1].p3;
  u3.p3 = pat.c[2].p3;
  u3.p4 = pat.c[3].p3;

  u4.p1 = pat.c[0].p4;
  u4.p2 = pat.c[1].p4;
  u4.p3 = pat.c[2].p4;
  u4.p4 = pat.c[3].p4;

  curve uc;
  uc.p1 = bezcurve(u1, v).p;
  uc.p2 = bezcurve(u2, v).p;
  uc.p3 = bezcurve(u3, v).p;
  uc.p4 = bezcurve(u4, v).p;

  curvere rev = bezcurve(vc, v);
  curvere reu = bezcurve(uc, u);

  point normal = cross(reu.dev, rev.dev);
  float len = 1 / sqrt(sqr(normal.x) + sqr(normal.y) + sqr(normal.z));
  normal = sclar(normal, len);

  patchre pre;
  pre.p = rev.p;
  pre.nor = normal;

  return pre;
}

void subdividepatch(patch pat, float step){
  int numdiv = (1 / step);
  int iu, iv;
  patchre PASave[numdiv + 1][numdiv + 1];
  for(iu = 0; iu <= numdiv; iu++){
    float u = iu*step;
    for (iv = 0; iv <= numdiv; iv++){
      float v = iv*step;

      patchre pre = bezpatch(pat, u, v);
      PASave[iu][iv] = pre;
    }

  } if (wireframe == 1){
    if (ADP == false){
   for (iu = 0; iu < numdiv; iu++){
    for(iv = 0; iv < numdiv; iv++){
      point p1 = PASave[iu][iv].p;
      point p2 = PASave[iu + 1][iv].p;
      point p3 = PASave[iu + 1][iv + 1].p;
      point p4 = PASave[iu][iv + 1].p;

      point n1 = PASave[iu][iv].nor;
      point n2 = PASave[iu + 1][iv].nor;
      point n3 = PASave[iu + 1][iv + 1].nor;
      point n4 = PASave[iu][iv + 1].nor;

      glBegin(GL_QUADS);
      glNormal3f(n1.x, n1.y, n1.z);
      glVertex3f(p1.x ,p1.y, p1.z);
      glNormal3f(n2.x, n2.y, n2.z);
      glVertex3f(p2.x ,p2.y, p2.z);
      glNormal3f(n3.x, n3.y, n3.z);
      glVertex3f(p3.x ,p3.y, p3.z);
      glNormal3f(n4.x, n4.y, n4.z);
      glVertex3f(p4.x ,p4.y, p4.z);
      glEnd();
    }
  }
} else {
  for (iu = 0; iu < numdiv; iu++){
    for(iv = 0; iv < numdiv; iv++){
      point p1 = PASave[iu][iv].p;
      point p2 = PASave[iu + 1][iv].p;
      point p3 = PASave[iu + 1][iv + 1].p;
      point p4 = PASave[iu][iv + 1].p;

      point n1 = PASave[iu][iv].nor;
      point n2 = PASave[iu + 1][iv].nor;
      point n3 = PASave[iu + 1][iv + 1].nor;
      point n4 = PASave[iu][iv + 1].nor;

      glBegin(GL_TRIANGLES);
      glNormal3f(n1.x, n1.y, n1.z);
      glVertex3f(p1.x ,p1.y, p1.z);
      glNormal3f(n2.x, n2.y, n2.z);
      glVertex3f(p2.x ,p2.y, p2.z);
      glNormal3f(n3.x, n3.y, n3.z);
      glVertex3f(p3.x ,p3.y, p3.z);

      glNormal3f(n1.x, n1.y, n1.z);
      glVertex3f(p1.x ,p1.y, p1.z);
      glNormal3f(n4.x, n4.y, n4.z);
      glVertex3f(p4.x ,p4.y, p4.z);
      glNormal3f(n3.x, n3.y, n3.z);
      glVertex3f(p3.x ,p3.y, p3.z);
      glEnd();
    }
  }
}
} else {
  if (ADP == false){
  for (iu = 0; iu < numdiv; iu++){
    for(iv = 0; iv < numdiv; iv++){
      point p1 = PASave[iu][iv].p;
      point p2 = PASave[iu + 1][iv].p;
      point p3 = PASave[iu + 1][iv + 1].p;
      point p4 = PASave[iu][iv + 1].p;

      drawLine(p1, p2);
      drawLine(p2, p3);
      drawLine(p3, p4);
      drawLine(p4, p1);
    }
  }
} else {
  for (iu = 0; iu < numdiv; iu++){
    for(iv = 0; iv < numdiv; iv++){
      point p1 = PASave[iu][iv].p;
      point p2 = PASave[iu + 1][iv].p;
      point p3 = PASave[iu + 1][iv + 1].p;
      point p4 = PASave[iu][iv + 1].p;

      drawLine(p1, p2);
      drawLine(p2, p3);
      drawLine(p3, p4);
      drawLine(p4, p1);
      drawLine(p1, p3);
    }
  }
}
} 
}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
  viewport.w = w;
  viewport.h = h;

  glViewport(0,0,viewport.w,viewport.h);// sets the rectangle that will be the window
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();                // loading the identity matrix for the screen
  gluPerspective(45.0,                  //The camera angle
                   (double)w / (double)h, //The width-to-height ratio
                   1.0,                   //The near z clipping coordinate
                   200.0);                //The far z clipping coordinate

  //----------- setting the projection -------------------------
  // glOrtho sets left, right, bottom, top, zNear, zFar of the chord system


  // glOrtho(-1, 1 + (w-400)/200.0 , -1 -(h-400)/200.0, 1, 1, -1); // resize type = add
  // glOrtho(-w/400.0, w/400.0, -h/400.0, h/400.0, 1, -1); // resize type = center

  glOrtho(-1, 1, -1, 1, 1, -1);    // resize type = stretch

  //------------------------------------------------------------
}
void printpoint(point p){
  printf("%f %f %f\n", p.x, p.y, p.z);
}

//****************************************************
// sets the window up
//****************************************************
void initScene(){
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // Clear to black, fully transparent
  
  myReshape(viewport.w,viewport.h);
}


//***************************************************
// function that does the actual drawing
//***************************************************
void myDisplay() {


  //----------------------- ----------------------- -----------------------


  glClear(GL_COLOR_BUFFER_BIT);                // clear the color buffer (sets everything to black)


  GLfloat light_ambient[] = {0.5, 0.5, 0.2, 1.0};
  GLfloat light_diffuse[] = {1.0, 0.3, 0.6, 1.0};
  GLfloat light_specular[] = {0.0, 0.5, 1.0, 1.0};
  GLfloat light_position[] = {-50.0, -50.0, -70.0, 0.0};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  GLfloat light_position1[] = {50.0, 50.0, 70.0, 0.0};
  glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
  glEnable(GL_LIGHT1);

  GLfloat light_position2[] = {50.0, 50.0, -70.0, 0.0};
  glLightfv(GL_LIGHT2, GL_DIFFUSE, light_diffuse);
  glLightfv(GL_LIGHT2, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT2, GL_SPECULAR, light_specular);
  glLightfv(GL_LIGHT2, GL_POSITION, light_position2);
  glEnable(GL_LIGHT2);

  if (shade == 1){
    glShadeModel(GL_SMOOTH);
  } else{
    glShadeModel(GL_FLAT);
  }



  glMatrixMode(GL_MODELVIEW);                  // indicate we are specifying camera transformations
  glLoadIdentity();
  //glTranslatef(0.0f, 0.0f, -10.0f); 
  glTranslatef(tx, ty, tz);
  glRotatef (tippangle, 1,0,0);  // Up and down arrow keys 'tip' view.
  glRotatef (viewangle, 0,1,0);  // Right/left arrow keys 'turn' view.
  
  /*glPushMatrix();
  glRotatef(-70, 1, 0, 0);  */ 
  //GLfloat light_position[] = {0.0, 0.0, 0.0, 0.0};     
  //glLightfv(GL_LIGHT0, GL_POSITION, light_position);                      

  //----------------------- code to draw objects --------------------------
  

  /*int i, j;
  for (i = 0; i < numofpatch; i ++){
    for (j = 0; j < 4; j ++){
      point pold = PA[i].c[j].p1;
      glColor3f(0.0, 1.0, 1.0);
      for (float t = 0.0; t < 1.0; t += SUB){
        point p = bezcurve(PA[i].c[j], t).p;
        drawLine(pold, p);
        pold = p;
      }
    }
  }*/

    int i ;
    for (i = 0; i < numofpatch; i ++){
      glColor3f(0.0, 1.0, 1.0);
        subdividepatch(PA[i], SUB);
    }


   //glPopMatrix();

  //subdividepatch(PA[0], 0.1);
    //point pold = PA[0].c[0].p1;
    //point p = bezcurve(PA[0].c[0], SUB).p;
    //drawLine(pold, p);
    /*for (float t = 0.0; t < 1.0; t += SUB){
        point p = bezcurve(PA[0].c[0], t).p;
        drawLine(pold, p);
        pold = p;
      }*/
    /*point p1;
    p1.x = 1.4;
    p1.y = 0.0;
    p1.z = 2.4;
    point p2;
    p2.x = 1.0f;
    p2.y = 2.0f;
    p2.z = 3.0f;
    point r = cross(p1, p2);
    printpoint(r);*/
    /*glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_TRIANGLES); //Begin triangle coordinates
    
    //Pentagon
    glVertex3f(0.5f, 0.5f, 3.0f);
    glVertex3f(1.5f, 0.5f, 3.0f);
    glVertex3f(0.5f, 1.0f, 3.0f);
    glEnd();*/
    /*point p1;
    p1.x = 0.0;
    p1.y = 0.0;
    p1.z = 0.0;
    point p2;
    p2.x = 0.0f;
    p2.y = 0.5f;
    p2.z = 3.0f;*/
    //drawLine(p1, p2);



  //-----------------------------------------------------------------------

  glFlush();
  glutSwapBuffers();                           // swap buffers (we earlier set double buffer)
}

// key press function
void keypress(unsigned char key, int x, int y){
  switch(key) {
    case 'w':
    wireframe = 1 - wireframe;
    break;
    case 's':
    shade = 1 - shade;
    break;
    case '=':
    tz = tz - 1;
    break;
    case '-':
    tz = tz + 1;
    break;
  } glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y){
  int mod;
  switch(key){
    case GLUT_KEY_UP: 
    mod = glutGetModifiers();
    if (mod == GLUT_ACTIVE_SHIFT){
      ty += 0.5;
      break;
    }
    tippangle -= 5;
    break;
    case GLUT_KEY_DOWN:
    mod = glutGetModifiers();
    if (mod == GLUT_ACTIVE_SHIFT){
      ty -= 0.5;
      break;
    }
     tippangle += 5;
    break;
    case GLUT_KEY_LEFT:
    mod = glutGetModifiers();
    if (mod == GLUT_ACTIVE_SHIFT){
      tx -= 0.5;
      break;
    } viewangle -= 5;
    break;
    case GLUT_KEY_RIGHT:
    mod = glutGetModifiers();
    if (mod == GLUT_ACTIVE_SHIFT){
      tx += 0.5;
      break;
    } viewangle += 5;
    break;

  } glutPostRedisplay();
}


//****************************************************
// called by glut when there are no messages to handle
//****************************************************
void myFrameMove() {
  //nothing here for now
#ifdef _WIN32
  Sleep(10);                                   //give ~10ms back to OS (so as not to waste the CPU)
#endif
  glutPostRedisplay(); // forces glut to call the display function (myDisplay())
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
  //This initializes glut
  glutInit(&argc, argv);

  // set adaptive to false
  ADP = false;

  ifstream infile;
  infile.open(argv[1]);
  if (!infile.good()){
    return 1;
  }
  SUB = atof(argv[2]);
  if (argc > 3){
    char *ar3 = argv[3];
    if (strcmp(ar3, "-a")==0){
      ADP = true;
    }
  }

  char firststring[5];
  infile.getline(firststring, 5);
  numofpatch = atoi(firststring);
  PA = new patch[numofpatch];
  int i;

  for(i = 0; i < numofpatch; i ++){
    char empty[maxchar];
    int j;
    for (j = 0; j < 4; j++){
    char STR[maxchar];
    int n = 0;
    infile.getline(STR, maxchar);
    char *tokens[maxtoken] = {};
    tokens[0] = strtok(STR, DELIMITER);
    if (tokens[0]){
      for (n = 1; n < maxtoken; n++){
        tokens[n] = strtok(0, DELIMITER);
        if (!tokens[n]){
          break;
        }
      }
    }
    PA[i].c[j].p1.x = atof(tokens[0]);
    PA[i].c[j].p1.y = atof(tokens[1]);
    PA[i].c[j].p1.z = atof(tokens[2]);
    PA[i].c[j].p2.x = atof(tokens[3]);
    PA[i].c[j].p2.y = atof(tokens[4]);
    PA[i].c[j].p2.z = atof(tokens[5]);
    PA[i].c[j].p3.x = atof(tokens[6]);
    PA[i].c[j].p3.y = atof(tokens[7]);
    PA[i].c[j].p3.z = atof(tokens[8]);
    PA[i].c[j].p4.x = atof(tokens[9]);
    PA[i].c[j].p4.y = atof(tokens[10]);
    PA[i].c[j].p4.z = atof(tokens[11]);
  }
  infile.getline(empty, maxchar);
  } infile.close();

  //This tells glut to use a double-buffered window with red, green, and blue channels 
  glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);

  // Initalize theviewport size
  viewport.w = 400;
  viewport.h = 400;

  //The size and position of the window
  glutInitWindowSize(viewport.w, viewport.h);
  glutInitWindowPosition(0, 0);
  glutCreateWindow("CS184!");

  initScene();                                 // quick function to set up scene

  glutDisplayFunc(myDisplay);                  // function to run when its time to draw something
  glutReshapeFunc(myReshape);                  // function to run when the window gets resized
  glutIdleFunc(myFrameMove);                   // function to run when not handling any other task
  glutKeyboardFunc(keypress);
  glutSpecialFunc(SpecialKeys);
  glutMainLoop();                              // infinite loop that will keep drawing and resizing and whatever else

  return 0;
}








