/*

    Test some OpenGL.

    gcc cubeGL.c -o cubeGL -lglut -lGL -lGLU

*/


#include <time.h>
#include <GL/glut.h>

GLfloat angle= 0.0;
GLfloat bez_points[4][3] = {{ -1.0, -1.0, 1.0}, { 1.0, -1.0, 1.0}, {-1.0, 1.0, 1.0}, {1.0, 1.0, 1.0}};

void init(void)
 {
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &bez_points[0][0]);
   glEnable(GL_MAP1_VERTEX_3);
   glEnable(GL_LINE_SMOOTH);
 }
void drawString(void *font, float x, float y, float z, char *str)
  {
   char *ch=NULL;
   glRasterPos3f(x, y, z);
   for(ch= str; *ch; ch++)glutBitmapCharacter(font, (int)*ch);
  }
void spin(void)
  {
    angle+= 1.0;
    glutPostRedisplay();
  }
void display(void)
  {
    int i=0;
    time_t time1;
    struct tm* tm_info;
    char buffer[30];
    GLfloat side1=1.0;
    GLfloat side2=-1.0;
    glClearColor(0.0, 0.0, 0.3, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glLoadIdentity();
    gluLookAt(5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    time(&time1);
    tm_info = localtime(&time1);
    strftime(buffer, 25, "%m-%d-%Y %I:%M:%S\n", tm_info);
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, 1.0, 2.0, 0.0, buffer);

    glRotatef(angle, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    glRotatef(angle, 0, 0, 1);

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(side2, side2, side1);
    glVertex3f(side1, side2, side1);
    glVertex3f(side1, side1, side1);
    glVertex3f(side2, side1, side1);
    glEnd();

    glColor3f(1.0, 1.0, 1.0);
    drawString(GLUT_BITMAP_TIMES_ROMAN_24, 1.0, 1.0, 1.0, "_____Bezier Curve");

    glLineWidth(5.0);
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_STRIP);
      for (i = 0; i <= 20; i++) 
         glEvalCoord1f((GLfloat) i/20.0);
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(side2, side1, side1);
    glVertex3f(side1, side1, side1);
    glVertex3f(side1, side1, side2);
    glVertex3f(side2, side1, side2);
    glEnd();

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(side2, side1, side2);
    glVertex3f(side1, side1, side2);
    glVertex3f(side1, side2, side2);
    glVertex3f(side2, side2, side2);
    glEnd();

    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(side2, side2, side2);
    glVertex3f(side1, side2, side2);
    glVertex3f(side1, side2, side1);
    glVertex3f(side2, side2, side1);
    glEnd();

    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(side1, side2, side1);
    glVertex3f(side1, side2, side2);
    glVertex3f(side1, side1, side2);
    glVertex3f(side1, side1, side1);
    glEnd();

    glColor3f(0.0, 1.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(side2, side2, side2);
    glVertex3f(side2, side2, side1);
    glVertex3f(side2, side1, side1);
    glVertex3f(side2, side1, side2);
    glEnd(); 
 
    glutSwapBuffers();
  }
void reshape(int w, int h)
  {
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w/(GLfloat)h, 1.0, 10.0);
    glMatrixMode(GL_MODELVIEW);
   }
void keyboard(unsigned char key, int x, int y)
  {
    //escape key
    if (key==27)exit(0);
  }
int main(int argc, char **argv)
  {
    glutInit(&argc, argv);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition (100, 100);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("cubeGL");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutIdleFunc(spin); 
  
    glutMainLoop();
    return 0;
  }

