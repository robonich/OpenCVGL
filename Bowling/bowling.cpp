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
void glut_timer(int value);
void glut_keyboardup(unsigned char key, int x, int y);

class Sphere//sphere class
{
public:
	double radius;
	double mass;
	std::vector<double> pos;//pos size is 3
	std::vector<double> velocity = {0,0,0};// velocity size is 3
	Sphere *back_up;
	bool hasChild;

	enum DRAW_TYPE { SOLID = 0, WIRE = 1};

	Sphere();
	Sphere(double _radius, double _mass, std::vector<double> _pos);
	Sphere(double _radius, double _mass, std::vector<double> _pos, bool _hasChild);
	~Sphere(){ if(hasChild) delete back_up; }//もしバックアップを持っていたら、子供の領域を開放する

	void addForce(double x, double y, double z, double force);//ある方向に力を加える
	void addForce(std::vector<double> axis, double force);//axisはsize 3

	void updatePos();//速度に応じて現在座標を移動させる

	void reset();//初期位置に戻す

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

	bool isTouchedSphere(Sphere &s);

	void draw();
};

class Pin //pin class //接触判定は円柱で行う
{
public:
	std::vector<double> pos;
	std::vector<double> velocity = {0,0,0};
	double radius;
	double height;
	double mass;
	Pin *back_up;
	bool hasChild;
	GLUquadricObj *cylinder;//円柱を描くために必要なもの

	Pin();
	Pin(std::vector<double> _pos, double _radius, double _height, double _mass);
	Pin(std::vector<double> _pos, double _radius, double _height, double _mass, bool _hasChild);
	~Pin(){ if(hasChild) delete back_up; gluDeleteQuadric(cylinder);};

	void updatePos();
	void draw();
	void reset();

	bool isBottonOnFloor();
	bool isTouchedPin();
	bool isTouchedBowl(Sphere &s);

	void collisionReactionWith(Sphere &s);
	void collisionReactionWith(Pin &s);
};

Sphere bowl(0.5, 0.5, {0.0,0.5,0.0}, true);
Floor floor1({5,30}, {0,0,-15}, 0);
Pin pin1[] = { Pin({1,1.5,-25}, 0.3, 1.5, 0.3, true),
						  Pin({0,1.5,-22}, 0.3, 1.5, 0.3, true),
						  Pin({1,1.5,-19}, 0.3, 1.5, 0.3, true),
						  Pin({0,1.5,-16}, 0.3, 1.5, 0.3, true),
						  Pin({3,1.5,-25}, 0.3, 1.5, 0.3, true),
						  Pin({-3,1.5,-25}, 0.3, 1.5, 0.3, true),
						  Pin({-1,1.5,-25}, 0.3, 1.5, 0.3, true),
						  Pin({-2,1.5,-22}, 0.3, 1.5, 0.3, true),
						  Pin({2,1.5,-22}, 0.3, 1.5, 0.3, true),
						  Pin({-1,1.5,-19}, 0.3, 1.5, 0.3, true)
						};

bool upkeydown = false; //false: key up, true: key down
bool downkeydown = false;
bool leftkeydown = false;
bool rightkeydown = false;

bool iskeydown[127], iskeyup[127];

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
	for(int i = 0; i < 127; i++){
		iskeydown[i] = false;
		iskeyup[i] = false;
	}
	glClearColor(0.0, 0.0, 0.0, 0.0);
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);
	glutKeyboardFunc(glut_keyboard);
	glutKeyboardUpFunc(glut_keyboardup);
	glutSpecialFunc(specialkeydown);
	glutTimerFunc(1.0/DT, glut_timer, 0);
}

void glut_keyboard(unsigned char key, int x, int y){
	switch(key){
		case 'q':
		case 'Q':
		case '\033':
		exit(0);
		break;
		case 'w':
		case 'W':
		iskeydown['w'] = true;
		iskeyup['w'] = false;
		break;
		case 'a':
		case 'A':
		iskeydown['a'] = true;
		iskeyup['a'] = false;
		break;
		case 's':
		case 'S':
		iskeydown['s'] = true;
		iskeyup['s'] = false;
		break;
		case 'd':
		case 'D':
		iskeydown['d'] =true;
		iskeyup['d'] = false;
		break;
		case 'r':
		bowl.reset();
		for(auto &i : pin1) i.reset();
		break;
	}
	glutPostRedisplay();
}

void glut_keyboardup(unsigned char key, int x, int y){
	iskeyup[key] = true;
	iskeydown[key] = false;
}

void specialkeydown(int key, int x, int y)
{
     if( key == GLUT_KEY_UP )//矢印「上」
     {
     	upkeydown = true;
     }

     if( key == GLUT_KEY_DOWN )//矢印「下」
     {
     	downkeydown = true;
     }

     if( key == GLUT_KEY_LEFT )
     {
     	leftkeydown = true;
     }

     if( key == GLUT_KEY_RIGHT )
     {
     	rightkeydown = true;
     }
     glutPostRedisplay();
 }

 void glut_timer(int value){
 	if(iskeydown['w']) {bowl.addForce({0.0, 0.0, -1.0}, 1.0); /*upkeydown = false;*/}
 	if(iskeydown['s']) {bowl.addForce({0.0, 0.0, 1.0}, 1.0); /*downkeydown = false;*/}
 	if(iskeydown['a']) {bowl.addForce({-1.0, 0.0, 0.0}, 1.0); /*leftkeydown = false;*/}
 	if(iskeydown['d']) {bowl.addForce({1.0, 0.0, 0.0}, 1.0); /*rightkeydown = false;*/}
 	bowl.updatePos();

 	for(auto &i : pin1){
 		i.collisionReactionWith(bowl);
 		i.updatePos();
 		// std::cout << pin1.isTouchedBowl(bowl) << "\n";
	}
 	// std::cout << bowl.pos[0] << "\t" << bowl.pos[1] << "\t" << bowl.pos[2] << "\t" << floor1.pos[0] << "\t" << floor1.pos[1] << "\t" << floor1.pos[2] << "\t" << floor1.isTouchedSphere(bowl) <<"\n";
 	if(!floor1.isTouchedSphere(bowl)) bowl.reset();
 	glutPostRedisplay();
 	glutTimerFunc(1.0/DT , glut_timer , 0);
 }

 void glut_display(){
 	glMatrixMode(GL_PROJECTION);
 	glLoadIdentity();
 	gluPerspective(30.0, 1.0, 0.1, 100);

 	glMatrixMode(GL_MODELVIEW);
 	glLoadIdentity();
 	gluLookAt(bowl.pos[0], bowl.pos[1]+10.0, bowl.pos[2]+20.0,
 		bowl.pos[0], bowl.pos[1], bowl.pos[2],
 		0.0, 1.0, 0.0);

 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 	glEnable(GL_DEPTH_TEST);

 	bowl.draw(Sphere::WIRE);
 	floor1.draw();
 	for(auto &i : pin1){
 		i.draw();
 	}

 	glFlush();
 	glDisable(GL_DEPTH_TEST);

 	glutSwapBuffers();
 }

 Sphere::Sphere() : Sphere(0.25, 0.1, {0,0,0}, false){}
 Sphere::Sphere(double _radius, double _mass, std::vector<double> _pos) : Sphere(_radius, _mass, _pos, false){}
 Sphere::Sphere(double _radius, double _mass, std::vector<double> _pos, bool _hasChild) : radius(_radius), mass(_mass), pos(_pos), hasChild(_hasChild){
 	if (hasChild){
 		back_up = new Sphere(_radius, _mass, _pos, false);
 	} else {
 		back_up = this;
 	}
 }

 void Sphere::addForce(double x, double y, double z, double force){
 	double mag = sqrt(x*x+y*y+z*z);
 	velocity[0] += force / mass * DT * x / mag;
 	velocity[1] += force / mass * DT * y / mag;
 	velocity[2] += force / mass * DT * z / mag;
}//ある方向に力を加える
void Sphere::addForce(std::vector<double> axis, double force){
	addForce(axis[0], axis[1], axis[2], force);
}
void Sphere::updatePos(){
	pos[0] += velocity[0] * DT;
	pos[1] += velocity[1] * DT;
	pos[2] += velocity[2] * DT;
}

void Sphere::draw(DRAW_TYPE d){
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

void Sphere::reset(){
	*this = *back_up;
}

Floor::Floor() : Floor({0,0}, {0,0,0}, 0){}
Floor::Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric) : 
size(_size), pos(_pos), coe_fric(_coe_fric) {}

bool Floor::isTouchedSphere(Sphere &s){//このようにsphereを渡すときは参照渡しにしないといけない。じゃないと呼ばれるたびにデストラクタが呼ばれてエラーが起きてしまう。
	if(s.pos[0] > pos[0]+0.5*size[0] || s.pos[0] < pos[0]-0.5*size[0]) return false;
	if(s.pos[2] > pos[2]+0.5*size[1] || s.pos[2] < pos[2]-0.5*size[1]) return false;
	if(s.pos[1] - pos[1] < s.radius+0.1 && s.pos[1] - pos[1] > s.radius-0.1) return true;
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

Pin::Pin() : Pin({0,1,-15}, 0.3, 2, 0.3, false){}
Pin::Pin(std::vector<double> _pos, double _radius, double _height, double _mass) : Pin(_pos, _radius, _height, _mass, false){}
Pin::Pin(std::vector<double> _pos, double _radius, double _height, double _mass, bool _hasChild) : 
pos(_pos), radius(_radius), height(_height), mass(_mass), hasChild(_hasChild) {
	if (hasChild){
		back_up = new Pin(_pos, _radius, _height, _mass, false);
	} else {
		back_up = this;
	}
	cylinder = gluNewQuadric();
}



void Pin::updatePos(){
	pos[0] += velocity[0] * DT;
	pos[1] += velocity[1] * DT;
	pos[2] += velocity[2] * DT;
}
void Pin::draw(){
	glPushMatrix();
	glColor3d(1.0,0,1.0);
	glTranslatef(pos[0], pos[1], pos[2]);
	glRotatef(90,1,0,0);//degree, x, y, z
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, radius, radius, height, 30, 30);
	glPopMatrix();
}

bool Pin::isBottonOnFloor(){}
bool Pin::isTouchedPin(){

}
bool Pin::isTouchedBowl(Sphere &s){
	double dx = abs(pos[0]-s.pos[0]);
	double dy = abs(pos[2]-s.pos[2]);
	double dist = sqrt(dx*dx + dy*dy);
	if(dist < radius + s.radius) return true;
	else return false;
}
void Pin::collisionReactionWith(Sphere &s){
	if(isTouchedBowl(s)){
		std::vector<double> direction;

		velocity[0] = s.mass / mass * s.velocity[0];
		velocity[1] = s.mass / mass * s.velocity[1];
		velocity[2] = s.mass / mass * s.velocity[2];
	}
}
void Pin::reset(){
	*this = *back_up;
}

void Pin::collisionReactionWith(Pin &s){

}