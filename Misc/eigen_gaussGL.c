/*

    Test code for some eigen vectors and values. Try Gaussian distribution. Left click
to stop rotation.

    gcc -Wall -std=c99 eigen_gaussGL.c -o eigen_gaussGL -lglut -lGL -lGLU -lm -lgsl -lgslcblas

    C. Eric Cashon

*/

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <GL/glut.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_randist.h>


static GLfloat angle= 0.0;
static bool rotate=true;
static gsl_matrix *test_data_points= NULL;
static double test_data_means[3];
static double eigen_vectors[3][3];
static double eigen_values[3];
static double eigen_distance[3];
static int records=1000;


void get_data_points(int records)
 {
   int i=0;
   int j=0;
   const gsl_rng_type *T;
   gsl_rng *r;
   gsl_rng_env_setup();
   T = gsl_rng_mt19937;
   r = gsl_rng_alloc(T);
   //Global
   test_data_points=gsl_matrix_alloc(records, 3);
    
   //Get some test data.
   for(i=0; i<records; i++ )
    {
      for(j=0;j<3;j++)
         {
           if(j==0)gsl_matrix_set(test_data_points, i, j, gsl_ran_gaussian(r, 2.0));
           else gsl_matrix_set(test_data_points, i, j, gsl_ran_gaussian(r, 1.0));
           test_data_means[j]+=gsl_matrix_get(test_data_points, i, j);
         }
    }

   //Get means.
   for(i=0;i<3;i++)
     {
       test_data_means[i]=test_data_means[i]/records;
     }
   printf("Means %f, %f, %f\n", test_data_means[0], test_data_means[1], test_data_means[2]);

   //Get covariance matrix.
   gsl_matrix *covariance= gsl_matrix_alloc(3, 3);
   gsl_blas_dgemm(CblasTrans, CblasNoTrans, 1.0, test_data_points, test_data_points, 0, covariance);
   gsl_matrix_scale(covariance, 1.0/records);

   printf("Covariance matrix\n");
   for(i=0;i<3;i++)
      {
        for(j=0;j<3;j++)
           {
             printf("%f ", gsl_matrix_get(covariance, i, j));
           }
        printf("\n");
      }

   gsl_vector *eval = gsl_vector_alloc(3);
   gsl_matrix *evec = gsl_matrix_alloc(3, 3);
   gsl_eigen_symmv_workspace *w = gsl_eigen_symmv_alloc(3);
   gsl_eigen_symmv(covariance, eval, evec, w);
   //gsl_eigen_symmv_sort(eval, evec, GSL_EIGEN_SORT_ABS_DESC);
   gsl_eigen_symmv_free(w);
   for(i = 0; i < 3; i++)
      {
        printf ("eigenvalue = %f\n", gsl_vector_get(eval, i));
        eigen_values[i]=gsl_vector_get(eval, i);
        printf ("eigenvector = %f, %f, %f\n", gsl_matrix_get(evec, i, 0), gsl_matrix_get(evec, i, 1), gsl_matrix_get(evec, i, 2) );
        for(j=0;j<3;j++)
           {
             eigen_vectors[i][j]=gsl_matrix_get(evec, i, j);
           }
      }

   //Get distance
   double distance=0;
   double x1=0;
   double y1=0;
   double z1=0;
   distance= sqrt(gsl_cdf_chisq_Qinv(0.05, 3));
   x1=distance*sqrt(gsl_vector_get(eval, 0));
   y1=distance*sqrt(gsl_vector_get(eval, 1));
   z1=distance*sqrt(gsl_vector_get(eval, 2));
   printf("distance %f, x1 %f, y1 %f, z1 %f\n", distance, x1, y1, z1);
   eigen_distance[0]=x1;
   eigen_distance[1]=y1;
   eigen_distance[2]=z1;
   
   gsl_vector_free(eval);
   gsl_matrix_free(evec);
   gsl_matrix_free(covariance);
   gsl_rng_free(r);

 }
void init(void)
 {
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   glEnable(GL_LINE_SMOOTH);
 }
void spin(void)
  {
    if(rotate==true) angle+=1.0;
    glutPostRedisplay();
  }
void display(void)
  {
    glClearColor(0.0, 0.0, 0.0, 0.0); 
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(9.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glRotatef(angle, 0.0, 1.0, 0.0);
  
    //Axis lines
    glLineWidth(7.0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(4.0, 0.0, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 4.0, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 4.0);
    glEnd();
    
    //Draw test points.
    int i=0;
    glPointSize(5.0);
    glBegin(GL_POINTS);
    glColor4f(0.0, 1.0, 0.0, 1.0);
    for(i=0; i<records; i++ )
    {
        glVertex3f(gsl_matrix_get(test_data_points, i, 0), gsl_matrix_get(test_data_points, i, 1), gsl_matrix_get(test_data_points, i, 2));
    }
    glEnd();

    //Draw eigen vectors scaled by eigen values.
    glLineWidth(2.0);
    glTranslatef(test_data_means[0], test_data_means[1], test_data_means[2]);
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(eigen_vectors[0][0]*eigen_distance[0], eigen_vectors[0][1]*eigen_distance[0],eigen_vectors[0][2]*eigen_distance[0]);
    glEnd();

    glColor3f(1.0, 0.2, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(eigen_vectors[1][0]*eigen_distance[1], eigen_vectors[1][1]*eigen_distance[1],eigen_vectors[1][2]*eigen_distance[1]);
    glEnd();

    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(eigen_vectors[2][0]*eigen_distance[2], eigen_vectors[2][1]*eigen_distance[2], eigen_vectors[2][2]*eigen_distance[2]);
    glEnd();

    //Add a sphere
    glColor3f(0.2, 0.2, 0.2);
    glScalef(eigen_distance[0], eigen_distance[1], eigen_distance[2]);
    glutWireSphere(1.0, 30, 30);
   
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
void stop_rotation(int button, int state, int x, int y)
  {
    if(button==GLUT_LEFT_BUTTON)
      { 
        if(state==GLUT_DOWN)
          {
            if(rotate==true) rotate=false;
            else rotate=true;
          }
      }
  }
int main(int argc, char **argv)
  {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 500);
    glutInitWindowPosition (100, 100);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutCreateWindow("eigen_gaussGL");
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(stop_rotation);
    glutIdleFunc(spin); 

    //Get some random data points.
    get_data_points(records);
  
    glutMainLoop();
    return 0;
  }

