#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <math.h>

#define WINDOW_X (500)
#define WINDOW_Y (500)
#define  WINDOW_NAME "bowling"

#define DT 0.05 //dt

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void specialkeydown(int key, int x, int y);
//void glut_timer(int value);

class Sphere//sphere class
{
public:
	double radius;
  double mass;
	//double pos[3];
	std::vector<double> pos;//pos size is 3

	enum DRAW_TYPE { SOLID = 0, WIRE = 1};

	Sphere();
  Sphere(double _radius, double _mass, std::vector<double> _pos);

	void addForce(double x, double y, double z, double force);//ある方向に力を加える
	void addForce(std::vector<double> axis, double force);//axisはsize 3

	void draw(DRAW_TYPE d);
};

class Floor//floor class
{
public:
	std::vector<double> size;//size size is 2
	std::vector<double> pos;//pos size is 3
	double coe_fric; //摩擦係数0<=coe<=1

	Floor();
	Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric);

	bool isTouchedSphere(Sphere s);

	void draw();
};

Sphere bowl(1.0, 0.5, {0,0.5,0});
Floor floor1({2,10}, {0,0,0}, 0);

bool upkeydown = false; //false: key up, true: key down
bool downkeydown = false;
bool leftkeydown = false;
bool rightkeydown = false;

int main(int argc, char *argv[]){
	init_GL(argc, argv);

	init();

	set_callback_functions();

	glutMainLoop();

	return 0;
}

void init_GL(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_X, WINDOW_Y);
	glutCreateWindow(WINDOW_NAME);
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutSpecialFunc(specialkeydown);
	//glutTimerFunc(1.0/DT, glut_timer, 0);
}

void glut_keyboard(unsigned char key, int x, int y){
	switch(key){
		case 'q':
		case 'Q':
		case '\033':
		exit(0);
	}
	glutPostRedisplay();
}

void specialkeydown(int key, int x, int y)
{
     if( key == GLUT_KEY_UP )//矢印「上」
     {
       bowl.addForce({0.0, 0.0, -1.0}, 2.0);
       upkeydown = true;
     }

     if( key == GLUT_KEY_DOWN )//矢印「下」
     {
       bowl.addForce({0.0, 0.0, 1.0}, 2.0);
       downkeydown = true;
     }

     if( key == GLUT_KEY_LEFT )
     {
       bowl.addForce({-1.0, 0.0, 0.0}, 2.0);
       leftkeydown = true;
     }

     if( key == GLUT_KEY_RIGHT )
     {
       bowl.addForce({1.0, 0.0, 0.0}, 2.0);
       rightkeydown = true;
     }
     glutPostRedisplay();
}



/*void glut_timer(int value){
  if(upkeydown) bowl.addForce({0.0, 0.0, -1.0}, 2.0);
  if(downkeydown) bowl.addForce({0.0, 0.0, 1.0}, 2.0);
  if(leftkeydown) bowl.addForce({-1.0, 0.0, 0.0}, 2.0);
  if(rightkeydown) bowl.addForce({1.0, 0.0, 0.0}, 2.0);
}*/

void glut_display(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0.0, 10.0, 20.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	bowl.draw(Sphere::WIRE);
	floor1.draw();
	//glutWireSphere(0.5, 50, 50);

	glFlush();
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

Sphere::Sphere() : Sphere(0.25, 0.1, {0,0,0}){}
Sphere::Sphere(double _radius, double _mass, std::vector<double> _pos) : radius(_radius), mass(_mass), pos(_pos){}

void Sphere::addForce(double x, double y, double z, double force){
	double mag = sqrt(x*x+y*y+z*z);
	pos[0] += 0.5 * force / mass * DT * DT * x / mag;
	pos[1] += 0.5 * force / mass * DT * DT * y / mag;
	pos[2] += 0.5 * force / mass * DT * DT * z / mag;
}//ある方向に力を加える
void Sphere::addForce(std::vector<double> axis, double force){
	addForce(axis[0], axis[1], axis[2], force);
}
void Sphere::draw(/*Sphere::*/DRAW_TYPE d){
	glPushMatrix();
	glColor3d(1.0, 1.0, 1.0);
	glTranslatef(pos[0], pos[1], pos[2]);
	switch(d){
		case SOLID:
		glutSolidSphere(radius, 40, 40);
		case WIRE:
		glutWireSphere(radius, 40, 40);
	}
	glPopMatrix();
}

Floor::Floor() : Floor({0,0}, {0,0,0}, 0){}
Floor::Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric) : 
size(_size), pos(_pos), coe_fric(_coe_fric) {}

bool Floor::isTouchedSphere(Sphere s){
	double dx = pos[0] - s.pos[0];
	double dy = pos[1] - s.pos[1];
	double dz = pos[2] - s.pos[2];
	double distance = sqrt(dx*dx + dy*dy + dz*dz);
	if(distance > s.radius - 0.01 && distance < s.radius + 0.01) return true;
	else return false;
}

void Floor::draw(){
	glColor3d(1.0, 1.0, 0.0);
	glBegin(GL_POLYGON);
	glVertex3d(pos[0]+0.5*size[0], pos[1], pos[2]+0.5*size[1]);
	glVertex3d(pos[0]+0.5*size[0], pos[1], pos[2]-0.5*size[1]);
	glVertex3d(pos[0]-0.5*size[0], pos[1], pos[2]-0.5*size[1]);
	glVertex3d(pos[0]-0.5*size[0], pos[1], pos[2]+0.5*size[1]);
	glEnd();
}
