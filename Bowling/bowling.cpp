#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>
#include <math.h>
#include <opencv2/opencv.hpp>
#include <thread>
#include <pthread.h>
#include <iostream>
#include <chrono>

#define WINDOW_X (500)
#define WINDOW_Y (500)
#define  WINDOW_NAME "bowling"
#define DT 0.05 //dt

//openGL
void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void specialkeydown(int key, int x, int y);
void glut_timer(int value);
void glut_keyboardup(unsigned char key, int x, int y);
static void drawString(void *font, std::string str, int w, int h, int x0, int y0);
void set_texture();

void resetCVMode();

//openCV
void openCV_main();
void face_and_body_detection();
void detect_move();
void detect_face_slope();

void cv_idle();

//mutex
pthread_mutex_t mCVmode;

//other function
double get_costheta_of_vectors(std::vector<double> a, std::vector<double> b);

//class
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



class Pin //pin class //接触判定は円柱で行う
{
public:
	bool isRotated = false;
	bool degreeflag = false;
	double degree = 0;
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

	void addForce(double x, double y, double z, double force);
	void addForce(std::vector<double> axis, double force);

	void updatePos();
	void draw();
	void reset();
	void fadeout();

	bool isBottonOnFloor();
	bool isTouchedPin(Pin &p);
	bool isTouchedBowl(Sphere &s);

	void collisionReactionWith(Sphere &s);
	void collisionReactionWith(Pin &s);
};
class Floor//floor class
{
public:
	std::vector<double> size;//size size is 2
	std::vector<double> pos;//pos size is 3
	std::vector<double> color;
	double coe_fric; //摩擦係数0<=coe<=1

	Floor();
	Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric);
	Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric, std::vector<double> color);

	bool isTouchedSphere(Sphere &s);
	bool isTouchedPinBottom(Pin &p);
	bool isPinRotated(Pin &p);

	void draw();
};

//OpenCV global variable
#define FLOOR_L 70
#define FLOOR_W 7

int CVmode = -1;
cv::CascadeClassifier cascadeFace, cascadeUpbody;
cv::Mat frame, preFrame;
cv::VideoCapture cap;

bool retry = false;
bool first = true;

double angularVelocity;//首の回転から得られる角速度。これを球に反映させる
double throwDirection[3];//ボールを投げるときの力。ただしy方向は常に0;
double throwForce;
double throwPosX;

//GL gloval variable
#define TEXTURE_HEIGHT (512)
#define TEXTURE_WIDTH (512)
std::vector<std::string> inputFileNames = {"lane.jpg"};
GLuint g_TextureHandles[] = {0};

bool first_throw = false;
bool second_throw = false;
bool spare = false;
bool result = false;

int point = 0;

int gameButton = 0;

int phase = 0;

Sphere bowl(0.5, 5.8967, {0.0,0.5,0.0}, true);

Floor floor1({FLOOR_W,FLOOR_L}, {0,0,-0.5*FLOOR_L}, 0.3);

Floor garter1[] = { 
	Floor({1, FLOOR_L}, {-(FLOOR_W*0.5+0.5),-0.5,-0.5*FLOOR_L}, 0, {0.5, 0.3, 0.2}),
	Floor({1, FLOOR_L}, {(FLOOR_W*0.5+0.5),-0.5,-0.5*FLOOR_L}, 0, {0.5, 0.3, 0.2})
};

Pin pin1[] = {
	Pin({1,1.5,-FLOOR_L+5}, 0.3, 1.5, 1.6, true),
	Pin({0,1.5,-FLOOR_L+8}, 0.3, 1.5, 1.6, true),
	Pin({1,1.5,-FLOOR_L+11}, 0.3, 1.5, 1.6, true),
	Pin({0,1.5,-FLOOR_L+14}, 0.3, 1.5, 1.6, true),
	Pin({3,1.5,-FLOOR_L+5}, 0.3, 1.5, 1.6, true),
	Pin({-3,1.5,-FLOOR_L+5}, 0.3, 1.5, 1.6, true),
	Pin({-1,1.5,-FLOOR_L+5}, 0.3, 1.5, 1.6, true),
	Pin({-2,1.5,-FLOOR_L+8}, 0.3, 1.5, 1.6, true),
	Pin({2,1.5,-FLOOR_L+8}, 0.3, 1.5, 1.6, true),
	Pin({-1,1.5,-FLOOR_L+11}, 0.3, 1.5, 1.6, true)
};

bool upkeydown = false; //false: key up, true: key down
bool downkeydown = false;
bool leftkeydown = false;
bool rightkeydown = false;

bool iskeydown[127], iskeyup[127];

int main(int argc, char *argv[]){
	std::thread th1(openCV_main);

	init_GL(argc, argv);

	init();

	set_callback_functions();

	glutMainLoop();

	th1.join();

	return 0;
}

void init_GL(int argc, char *argv[]){
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(WINDOW_X, WINDOW_Y);
	glutCreateWindow(WINDOW_NAME);
}

void init(){
// 	glGenTextures(3, g_TextureHandles);

// 	for(int i = 0; i < 3; i++){
// 		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
// 		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
// 		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXTURE_WIDTH,
// 			TEXTURE_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
// 		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
// 	}
// 	set_texture();

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
		case 13://enter key
		if(CVmode == -1){
			if(gameButton == 0) {
				pthread_mutex_lock( &mCVmode);
				CVmode = 0;
				pthread_mutex_unlock( &mCVmode);
				first_throw = true;
			}
			if(gameButton == 1) exit(0);
		}
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
		if(gameButton == 0){
			gameButton = 1;
		} else {
			gameButton--;
		}
		leftkeydown = true;
	}

	if( key == GLUT_KEY_RIGHT )
	{
		if(gameButton == 1){
			gameButton = 0;
		} else {
			gameButton++;
		}
		rightkeydown = true;
	}
	glutPostRedisplay();
}

void glut_timer(int value){

	static bool addZForce = true;

	if(iskeydown['w']) {bowl.addForce({0.0, 0.0, -1.0}, 1.0); /*upkeydown = false;*/}
	if(iskeydown['s']) {bowl.addForce({0.0, 0.0, 1.0}, 1.0); /*downkeydown = false;*/}
	if(iskeydown['a']) {bowl.addForce({-1.0, 0.0, 0.0}, 1.0); /*leftkeydown = false;*/}
	if(iskeydown['d']) {bowl.addForce({1.0, 0.0, 0.0}, 1.0); /*rightkeydown = false;*/}


	if(phase == 0){
		if(!floor1.isTouchedSphere(bowl)){
			first_throw = false;
			second_throw = true;
			phase = 1;
			CVmode = 0;
			for(auto &p: pin1){
				if(floor1.isPinRotated(p) || !floor1.isTouchedPinBottom(p)){
					point++;
					p.fadeout();
				}
			}
			bowl.reset();
			
			addZForce = true;
		}
	} else if(phase == 1){
		if(!floor1.isTouchedSphere(bowl)){
			first_throw = false;
			second_throw = true;
			phase = 2;
			CVmode = 0;
			for(auto &p: pin1){
				std::cout << p.pos[0] << "," << p.pos[1]<< "," << p.pos[2] << std::endl;
				if (p.isRotated) continue;
				if(floor1.isPinRotated(p) || !floor1.isTouchedPinBottom(p)){
					point++;
					p.fadeout();
				}
			}
			bowl.reset();
			if(point == 10){
				spare = true;
			}
			addZForce = true;
		}
	} else if(phase == 2){
		result = true;
		second_throw = false;
		static int count = 0;
		count++;
		if(count > 500) {
			spare = false;
			bowl.reset();
			for(auto &p: pin1){
				p.reset();
			}
			CVmode = -1;
			phase = 1;
			count = 0;
			result = false;
			point = 0;
		}
	}

	// if(phase == 0){
	// 	glColor3d(0.0, 0.0, 1.0);
	// 	drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"FIRST THROW!!", WINDOW_X, WINDOW_Y, 10, 10);
	// 	if(!floor1.isTouchedSphere(bowl)) {
	// 		phase = 1;
	// 		CVmode = 0
	// 	}
	// }
	// if(phase == 1){
	// 	if(!floor1.isTouchedSphere(bowl)) {

	// 		resetCVMode();
	// 		CVmode = 0;
	// 		phase = 2;
	// 	}
	// 	if(point == 10){
	// 		glColor3d(0.0, 0.0, 1.0);
	// 		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"STRIKE!!", WINDOW_X, WINDOW_Y, 200, 300);
	// 		glColor3d(0.0, 1.0, 0);
	// 		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"STRIKE!!", WINDOW_X, WINDOW_Y, 205, 305);
	// 	} else {
	// 		glColor3d(0.0, 0.0, 1.0);
	// 		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"SECOND THROW!!", WINDOW_X, WINDOW_Y, 10, 10);
	// 	}
	// }
	// if(phase == 2){
	// 	resetCVMode();
	// }

	
	switch (CVmode){
		case -1:
		break;
		case 0:
		break;
		case 1:
		break;
		case 2:
		bowl.pos[0] = (double)throwPosX/640.0*FLOOR_W - FLOOR_W/2;
		break;
		case 3:
		if(bowl.pos[2] < -FLOOR_L/1.5){//摩擦がかかるところの区別
			bowl.addForce({1, 0, 0}, angularVelocity/80);
		}else{
			bowl.addForce({1, 0, 0}, angularVelocity/8000);
		}

		if(addZForce) {//げき力として与えるから最初の一回のみ
			bowl.addForce({throwDirection[0], 0, -2800}, (300 - throwForce)*3);
			addZForce = false;

			std::cout << CVmode << ", "
			<< angularVelocity << ", "
			<< throwPosX << ", "
			<< throwDirection[0] << ", "
			<< throwDirection[2] << ", "
			<< throwForce << std::endl;
		}
		break;
	}
	bowl.updatePos();


	for(int i = 0;i < 10; i++){
		for(int j = 0;j < 10; j++){
			if(i == j) continue;
			pin1[i].collisionReactionWith(pin1[j]);
		}
		pin1[i].collisionReactionWith(bowl);
		pin1[i].addForce({-pin1[i].velocity[0], -pin1[i].velocity[1], -pin1[i].velocity[2]}, floor1.coe_fric*pin1[i].mass);//摩擦力
		pin1[i].updatePos();
	}
	// std::cout << "pin1[9]" << pin1[9].pos[0] << "," << pin1[9].pos[2] << std::endl;
	// std::cout << "pin1[6]" << pin1[6].pos[0] << "," << pin1[6].pos[2] << std::endl;
	// std::cout << "dist" << sqrt((pin1[9].pos[0]-pin[6].pos[0])*(pin1[9].pos[0]-pin[6].pos[0])+(pin1[9].pos[2]-pin[6].pos[2])+pi


	glutPostRedisplay();
	glutTimerFunc(1.0/DT , glut_timer , 0);
}

void resetCVMode(){
	bowl.reset();
	for(auto &i: pin1){
		if(!floor1.isTouchedPinBottom(i) || floor1.isPinRotated(i)){
			point++;
			i.fadeout();
		} else {
			i.reset();
		}
	}
	angularVelocity = 0;
	throwDirection[0] = 0; throwDirection[1] = 0; throwDirection[2] = 0;
	throwPosX = 0;
	throwForce = 0;
	CVmode = -1;
}

static void drawString(void *font, std::string str, int w, int h, int x0, int y0)
{
	glDisable(GL_LIGHTING);
    // 平行投影にする
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, w, h, 0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

    // 画面上にテキスト描画
	glRasterPos2f(x0, y0);
	int size = (int)str.size();
	for(int i = 0; i < size; ++i){
		char ic = str[i];
		glutBitmapCharacter(font, ic);
	}

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void glut_display(){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(30.0, 1.0, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(bowl.pos[0], bowl.pos[1]+5.0, bowl.pos[2]+20.0,
		bowl.pos[0], bowl.pos[1], bowl.pos[2],
		0.0, 1.0, 0.0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	bowl.draw(Sphere::WIRE);
	for(auto &i: garter1){
		i.draw();
	}
	floor1.draw();
	for(auto &i : pin1){
		i.draw();
	}
	switch(CVmode){
		case -1:
		glColor3d(0.0, 0.0, 1.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"TAKU BOWL", WINDOW_X, WINDOW_Y, 200, 300);
		glColor3d(0.0, 1.0, 0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"TAKU BOWL", WINDOW_X, WINDOW_Y, 205, 305);

		if(gameButton == 0) {
			glColor3d(1.0, 0.0, 0.0);
			drawString(GLUT_BITMAP_HELVETICA_12, "START", WINDOW_X, WINDOW_Y, 150, 350);
			glColor3d(1.0, 1.0, 1.0);
			drawString(GLUT_BITMAP_HELVETICA_12, "END", WINDOW_X, WINDOW_Y, 325, 350);
		} else if(gameButton == 1){
			glColor3d(1.0, 1.0, 1.0);
			drawString(GLUT_BITMAP_HELVETICA_12, "START", WINDOW_X, WINDOW_Y, 150, 350);
			glColor3d(1.0, 0.0, 0.0);
			drawString(GLUT_BITMAP_HELVETICA_12, "END", WINDOW_X, WINDOW_Y, 325, 350);
		}
		
		break;
		case 0:
		break;
		case 1:
		break;
		case 2:
		break;
		case 3:
		break;
	}


	if(first_throw){
		glColor3d(1.0, 0.0, 0.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"FIRST THROW!!", WINDOW_X, WINDOW_Y, 40, 40);
	}

	if(second_throw){
		glColor3d(1.0, 0.0, 0.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"SECOND THROW!!", WINDOW_X, WINDOW_Y, 40, 40);
	}

	if(spare) {
		glColor3d(1.0, 1.0, 0.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"S P A R E!!", WINDOW_X, WINDOW_Y, 100, 80);	
	}

	if(result) {
		glColor3d(1.0, 0.0, 0.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 ,"R E S U L T :", WINDOW_X, WINDOW_Y, 40, 40);
	}

	if(CVmode != -1){
		std::string str = std::to_string(point);
		glColor3d(1.0, 0.0, 0.0);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 , "POINT = ", WINDOW_X, WINDOW_Y, 40, 100);
		drawString(GLUT_BITMAP_TIMES_ROMAN_24 , str, WINDOW_X, WINDOW_Y, 150, 100);
	}

	glFlush();
	glDisable(GL_DEPTH_TEST);

	glutSwapBuffers();
}

void set_texture(){
	for(int i=0; i<inputFileNames.size(); i++){
		cv::Mat input = cv::imread(inputFileNames[i], 1);
		// BGR -> RGBの変換

		// if(inputFileNames[i] == "lane.jpg"){
		// 	cv::resize
		// }
		cv::cvtColor(input, input, CV_BGR2RGB);

		glBindTexture(GL_TEXTURE_2D, g_TextureHandles[i]);
		glTexSubImage2D(GL_TEXTURE_2D, 0,
			(TEXTURE_WIDTH - input.cols) / 2.0,
			(TEXTURE_HEIGHT - input.rows) / 2.0,
			input.cols, input.rows,
			GL_RGB, GL_UNSIGNED_BYTE, input.data);
	} 
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

Floor::Floor() : Floor({0,0}, {0,0,0}, 0, {0,0,0}){}
Floor::Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric) : 
Floor(_size, _pos, _coe_fric, {1.0,1.0,0}){}
Floor::Floor(std::vector<double> _size, std::vector<double> _pos, double _coe_fric, std::vector<double> _color) :
size(_size), pos(_pos), coe_fric(_coe_fric), color(_color) {}

bool Floor::isTouchedSphere(Sphere &s){//このようにsphereを渡すときは参照渡しにしないといけない。じゃないと呼ばれるたびにデストラクタが呼ばれてエラーが起きてしまう。
	if(s.pos[0] > pos[0]+0.5*size[0] || s.pos[0] < pos[0]-0.5*size[0]) return false;
	if(s.pos[2] > pos[2]+0.5*size[1] || s.pos[2] < pos[2]-0.5*size[1]) return false;
	if(s.pos[1] - pos[1] < s.radius+0.1 && s.pos[1] - pos[1] > s.radius-0.1) return true;
	else return false;
}

bool Floor::isTouchedPinBottom(Pin &p){//底面に接しているか
	if(p.pos[0] > pos[0]+0.5*size[0] || p.pos[0] < pos[0]-0.5*size[0]) return false;
	if(p.pos[2] > pos[2]+0.5*size[1] || p.pos[2] < pos[2]-0.5*size[1]) return false;
	if(p.pos[1] - pos[1] == p.height) return true;
	else return false;
}

bool Floor::isPinRotated(Pin &p){
	if(p.degree >= 20) return true;
	else return false;
}

void Pin::addForce(double x, double y, double z, double force){
	if(velocity[0] == 0 && velocity[1] == 0 && velocity[2] == 0) return;
	double mag = sqrt(x*x+y*y+z*z);
	velocity[0] += force / mass * DT * x / mag;
	velocity[1] += force / mass * DT * y / mag;
	velocity[2] += force / mass * DT * z / mag;
}//ある方向に力を加える
void Pin::addForce(std::vector<double> axis, double force){
	addForce(axis[0], axis[1], axis[2], force);
}

void Floor::draw(){
	glColor3d(color[0], color[1], color[2]);
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
	if(degreeflag){
		glRotatef(degree+=5, -velocity[2], velocity[1], velocity[0]);//ぶつかった時の倒れる方向
		pos[1] -= pos[1]*sin(degree/360.0*M_PI) - radius * sin(degree/360.0*M_PI);
		// glTranslatef(0,pos[1]*sin(degree/360.0*M_PI),0);//高さの調整
	}
	if(degree >= 90) {
		degreeflag = false;
		glRotatef(degree, -velocity[2], velocity[1], velocity[0]);
	}
	gluQuadricDrawStyle(cylinder, GLU_FILL);
	gluCylinder(cylinder, radius, radius, height, 30, 30);
	glPopMatrix();
}

void Pin::fadeout(){
	isRotated = true;
	pos = {-100,-20,-100};
	velocity = {0,0,0};
}

bool Pin::isBottonOnFloor(){}
bool Pin::isTouchedPin(Pin &p){
	double dx = pos[0]-p.pos[0];
	double dz = pos[2]-p.pos[2];
	double dist = sqrt(dx*dx+dz*dz);
	if(dist <= radius + p.radius) return true;
	else return false;
}
bool Pin::isTouchedBowl(Sphere &s){
	double dx = pos[0]-s.pos[0];
	double dy = pos[2]-s.pos[2];
	double dist = sqrt(dx*dx + dy*dy);
	if(dist <= radius + s.radius) return true;
	else return false;
}
void Pin::collisionReactionWith(Sphere &s){
	if(isTouchedBowl(s)){
		std::vector<double> v_e = {s.pos[0]-pos[0], 0, s.pos[2]-pos[2]};//衝突軸ベクトル
		double mag_v = sqrt(v_e[0]*v_e[0]+v_e[1]*v_e[1]+v_e[2]*v_e[2]);
		v_e = {v_e[0]/mag_v, v_e[1]/mag_v, v_e[2]/mag_v};

		std::vector<double> vp_vs = {//vp-vs
			velocity[0]-s.velocity[0],
			velocity[1]-s.velocity[1],
			velocity[2]-s.velocity[2]
		};

		double Dot = v_e[0]*vp_vs[0] + v_e[1]*vp_vs[1] + v_e[2]*vp_vs[2];

		std::vector<double> c = {Dot*v_e[0], Dot*v_e[1], Dot*v_e[2]};

		double Mp = -2*s.mass/(mass+s.mass);
		double Ms = 2*mass/(mass+s.mass);

		velocity = {
			Mp*c[0]+velocity[0],
			Mp*c[1]+velocity[1],
			Mp*c[2]+velocity[2]
		};

		if(sqrt(velocity[0]*velocity[0]+velocity[1]*velocity[1]+velocity[2]*velocity[2]) > 3){
			degreeflag = true;
		}

		s.velocity = {
			Ms*c[0]+s.velocity[0],
			Ms*c[1]+s.velocity[1],
			Ms*c[2]+s.velocity[2]
		};
	}
}
void Pin::reset(){
	*this = *back_up;
}

void Pin::collisionReactionWith(Pin &s){
	if(isTouchedPin(s)){
		std::vector<double> v_e = {s.pos[0]-pos[0], 0, s.pos[2]-pos[2]};//衝突軸ベクトル
		double mag_v = sqrt(v_e[0]*v_e[0]+v_e[1]*v_e[1]+v_e[2]*v_e[2]);
		v_e = {v_e[0]/mag_v, v_e[1]/mag_v, v_e[2]/mag_v};

		std::vector<double> vp_vs = {//vp-vs
			velocity[0]-s.velocity[0],
			velocity[1]-s.velocity[1],
			velocity[2]-s.velocity[2]
		};

		double Dot = v_e[0]*vp_vs[0] + v_e[1]*vp_vs[1] + v_e[2]*vp_vs[2];

		std::vector<double> c = {Dot*v_e[0], Dot*v_e[1], Dot*v_e[2]};

		double Mp = -2*s.mass/(mass+s.mass);
		double Ms = 2*mass/(mass+s.mass);

		velocity = {
			Mp*c[0]+velocity[0],
			Mp*c[1]+velocity[1],
			Mp*c[2]+velocity[2]
		};

		if(sqrt(velocity[0]*velocity[0]+velocity[1]*velocity[1]+velocity[2]*velocity[2]) > 2){
			degreeflag = true;
		}

		s.velocity = {
			Ms*c[0]+s.velocity[0],
			Ms*c[1]+s.velocity[1],
			Ms*c[2]+s.velocity[2]
		};
	}
}

double get_costheta_of_vectors(std::vector<double> a, std::vector<double> b){
	double magA = 0, magB = 0, naiseki = 0;
	for(int i = 0; i < 3; i++){
		if(i == 1) continue;
		magA += a[i] * a[i];
		magB += b[i] * b[i];
		naiseki += a[i] * b[i];
	}
	magA = sqrt(magA); magB = sqrt(magB);
	return naiseki/magA/magB;
}

void openCV_main(){
	std::string  cascadeUpBodyFile = "/usr/local/share/OpenCV/haarcascades/haarcascade_upperbody.xml";
	std::string cascadeFaceFile = "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml";
	if(!cascadeFace.load(cascadeFaceFile)){printf("ERROR: cascadeFaceFile not found\n"); exit(0);}
	if(!cascadeUpbody.load(cascadeFaceFile/*cascadeUpBodyFile*/)){printf("ERROR: cascadeUpbodyFile not found\n"); exit(0);}

	cv::namedWindow("input", 1);
	cv::moveWindow("input", 500 , 200);
	cv::namedWindow("cutFace", 1);
	cv::namedWindow("mask", 1);
	cv::namedWindow("video", 1);

	cap.open(0);
	if(!cap.isOpened()){printf("Cannot open camera.\n"); exit(0);}

	cap>>frame;
	if(frame.empty()) exit(0);

	while(1){
		cap>>frame;
		if(frame.empty()) exit(0);
		cv::flip(frame, frame, 1);
		preFrame.create(frame.size(), CV_32FC3);
		frame.copyTo(preFrame);

		switch(CVmode){
			case -1:
			cv_idle();
			break;
			case 0://顔の左半面への移動検出
			detect_face_slope();
			break;
			case 1://顔の傾き検出
			face_and_body_detection();
			break;
			case 2://右手の移動検出
			detect_move();
			break;
			case 3://待機状態
			cv_idle();
		}
		auto key = cv::waitKey(5);
		if(key == 'q' || key == 'Q') break;
	}
	cv::destroyAllWindows();
	cv::destroyWindow("input");
	cv::destroyWindow("video");
	return;
}

void cv_idle(){//また投げる時まではただ映像を流し続ける
	cv::imshow("input", frame);
}

void detect_face_slope(){
	static bool detectSlopeFlag = false;

	double scale = 2.0;
	cv::Mat gray, smallImg(cv::saturate_cast<int>(frame.rows/scale), cv::saturate_cast<int>(frame.cols/scale), CV_8UC1);
	static auto start = std::chrono::system_clock::now();

	cv::cvtColor(frame, gray, CV_BGR2GRAY);
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	cv::equalizeHist(smallImg, smallImg);

	std::vector<cv::Rect> faces, upBodys;
	cascadeFace.detectMultiScale(smallImg, faces,
		1.1,
		2,
		CV_HAAR_SCALE_IMAGE);
	static cv::Point faceCenter;
	if(!detectSlopeFlag){
		if(faces.size() == 0) cv::line(frame, {320, 0}, {320, 480}, cv::Scalar(0,0,255), 5, 8, 0);
		for(int i = 0; i<(faces.size() != 0 ? 1 : 0); ++i){
			faceCenter.x = cv::saturate_cast<int>((faces[i].x + faces[i].width*0.5)*scale);
			faceCenter.y = cv::saturate_cast<int>((faces[i].y + faces[i].height*0.5)*scale);

			if(faceCenter.x > 290 &&faceCenter.x < 350 && faceCenter.y > 0){
				start = std::chrono::system_clock::now();
				detectSlopeFlag = true;	
				faceCenter.x = faceCenter.x / scale;
				faceCenter.y = faceCenter.y / scale;
			} 
			else cv::line(frame, {320, 0}, {320, 480}, cv::Scalar(0,0,255), 5, 8, 0);
		}
	} else {//顔が適正な位置に入ったら
		cv::line(frame, {320, 0}, {320, 480}, cv::Scalar(255), 5, 8, 0);//display blue line
		auto end = std::chrono::system_clock::now();
		auto diff = end - start;
		// std::cout << "elapsed time = "
		// << std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()
		// << " msec."
		// << std::endl;
		static bool rotateFlag = true;

		if(std::chrono::duration_cast<std::chrono::milliseconds>(diff).count()>2000 && rotateFlag){
			cv::Mat whiteFlash;
			whiteFlash.create(frame.size(),  CV_8UC1);
			whiteFlash = cv::Scalar(255);
			cv::Mat rotImg;
			int theta;
			std::vector<int> thetaBox;
			for (theta = -80; theta <= 80; theta+=5){
				cv::imshow("input", whiteFlash);
				cv::waitKey(5);
				cv::warpAffine(smallImg, rotImg, cv::getRotationMatrix2D(faceCenter, theta, 1.0), smallImg.size());
				cascadeFace.detectMultiScale(rotImg, faces,
					1.1,
					1,
					CV_HAAR_SCALE_IMAGE);
				if(faces.size() == 0)  continue;
				thetaBox.push_back(theta);
			}
			int avg_theta = 0;

			for(auto t: thetaBox){
				avg_theta += t/(int)thetaBox.size();
				std::cout << t <<  "\t" << avg_theta <<  "\t" << thetaBox.size() << "\n";
			}
			if(thetaBox.size() == 0){//角度の読み取りに失敗したらもう一度
				start = std::chrono::system_clock::now();
				printf("RETRY!!");
			}else{
				rotateFlag = false;
				pthread_mutex_lock( &mCVmode);
				CVmode = 1;
				detectSlopeFlag = false;
				rotateFlag = true;
				pthread_mutex_unlock( &mCVmode);

				angularVelocity = avg_theta;
			}
		}
	}
	cv::imshow("input", frame);
	return;
}
void face_and_body_detection(){
	static auto start = std::chrono::system_clock::now();
	static auto end = std::chrono::system_clock::now();

	double scale = 4.0;
	cv::Mat gray, smallImg(cv::saturate_cast<int>(frame.rows/scale), cv::saturate_cast<int>(frame.cols/scale), CV_8UC1);

	cv::cvtColor(frame, gray, CV_BGR2GRAY);
	cv::resize(gray, smallImg, smallImg.size(), 0, 0, cv::INTER_LINEAR);
	cv::equalizeHist(smallImg, smallImg);

	std::vector<cv::Rect> faces, upBodys;
	cascadeFace.detectMultiScale(smallImg, faces,
		1.1,
		1,
		CV_HAAR_SCALE_IMAGE);

	static cv::Point faceCenter;
	static cv::Point preFaceCenter(320,0);

	for(int i = 0; i<(faces.size() != 0 ? 1 : 0); ++i){
		int faceRadius, faceWidth, faceHeight;
		faceCenter.x = cv::saturate_cast<int>((faces[i].x + faces[i].width*0.5)*scale);
		faceCenter.y = cv::saturate_cast<int>((faces[i].y + faces[i].height*0.5)*scale);

		// cv::Rect roi_rect(faceCenter.x - faceWidth, faceCenter.y - faceHeight, faceWidth*2, faceHeight*2);
		// cv::Mat cutFace = frame(roi_rect);
		// cv::imshow("cutFace", cutFace);
	}
	if(faces.size() == 0){
		cv::line(frame, {preFaceCenter.x, 0}, {preFaceCenter.x, 480}, cv::Scalar(0,0,255), 5, 8, 0);	
	} else {
		cv::line(frame, {faceCenter.x, 0}, {faceCenter.x, 480}, cv::Scalar(255,0,0), 5, 8, 0);
		preFaceCenter = faceCenter;
	}

	end = std::chrono::system_clock::now();
	if(std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() > 3000){//三秒間の位置合わせ
		pthread_mutex_lock( &mCVmode);
		CVmode = 2; //move to next detection
		pthread_mutex_unlock( &mCVmode);
		start = std::chrono::system_clock::now();
		throwPosX = preFaceCenter.x;
	}
	cv::imshow("input", frame);
	return;
}

void detect_move(){//右反面だけ考えていれば良いから
	int INIT_TIME = 50;
	double B_PARAM = 1.0 / 50.0;
	double T_PARAM = 1.0 / 200.0;
	double Zeta = 10.0;

	cv::Rect detectMoveZone(320, 0 , 320, 320);//right half

	static bool init = false;

	static cv::Mat avg_img, sgm_img;
	static cv::Mat lower_img, upper_img, tmp_img;
	static cv::Mat dst_img, msk_img, bitwised_msk_img;

	static cv::Size s = frame.size();
	
	if(!init){
		avg_img.create(s, CV_32FC3);
		sgm_img.create(s, CV_32FC3);
		lower_img.create(s, CV_32FC3);
		upper_img.create(s, CV_32FC3);
		tmp_img.create(s, CV_32FC3);

		dst_img.create(s, CV_8UC3);
		msk_img.create(s, CV_8UC1);
		bitwised_msk_img.create(s, CV_8UC1);

		avg_img = cv::Scalar(0,0,0);

		for( int i = 0; i < INIT_TIME; i++){
			cap >> frame;
			if(frame.empty()) return;
			cv::flip(frame, frame, 1);
			
			cv::Mat linedFrame = frame.clone();
			cv::line(linedFrame, {160, 0}, {160, 480}, cv::Scalar(255,0,0), 5, 8, 0);
			cv::imshow("input", linedFrame);
			
			cv::Mat tmp;
			frame.convertTo(tmp, avg_img.type());
			cv::accumulate(tmp, avg_img);
			cv::waitKey(3);
		}

		avg_img.convertTo(avg_img, -1,1.0 / INIT_TIME);
		sgm_img = cv::Scalar(0,0,0);

		for( int i = 0; i < INIT_TIME; i++){
			cap >> frame;
			if(frame.empty()) return;
			cv::flip(frame, frame, 1);
			
			cv::Mat linedFrame = frame.clone();
			cv::line(linedFrame, {160, 0}, {160, 480}, cv::Scalar(255,0,0), 5, 8, 0);
			cv::imshow("input", linedFrame);

			frame.convertTo(tmp_img, avg_img.type());
			cv::subtract(tmp_img, avg_img, tmp_img);
			cv::pow(tmp_img, 2.0, tmp_img);
			tmp_img.convertTo(tmp_img, -1, 2.0);
			cv::sqrt(tmp_img, tmp_img);
			cv::accumulate(tmp_img, sgm_img);
			cv::waitKey(3);	
		}
		sgm_img.convertTo(sgm_img, -1, 1.0 / INIT_TIME);
		init = true;
		printf("first init\n");
	}

	frame.convertTo(tmp_img, tmp_img.type());

		// 4. check whether pixels are background or not
	cv::subtract(avg_img, sgm_img, lower_img);
	cv::subtract(lower_img, Zeta, lower_img);
	cv::add(avg_img, sgm_img, upper_img);
	cv::add(upper_img, Zeta, upper_img);
	cv::inRange(tmp_img, lower_img, upper_img, msk_img);

		// 5. recalculate 
	cv::subtract(tmp_img, avg_img, tmp_img);
	cv::pow(tmp_img, 2.0, tmp_img);
	tmp_img.convertTo(tmp_img, -1, 2.0);
	cv::sqrt(tmp_img, tmp_img);

		// 6. renew avg_img and sgm_img
	cv::accumulateWeighted(frame, avg_img, B_PARAM, msk_img);
	cv::accumulateWeighted(tmp_img, sgm_img, B_PARAM, msk_img);

	cv::bitwise_not(msk_img, msk_img);
	cv::accumulateWeighted(tmp_img, sgm_img, T_PARAM, msk_img);

	dst_img = cv::Scalar(0);
	frame.copyTo(dst_img, msk_img);

	cv::medianBlur(msk_img, msk_img, 5);
	// cv::fastNlMeansDenoising(msk_img, msk_img, 3, 7, 21);
	const cv::Point a(350, 80);
	const cv::Point b(640, 400);
	static cv::Point bottomPos(0, b.y), upPos(0, a.y);
	std::vector<int> posXOfWhiteOfBottom;
	std::vector<int> posXOfWhiteOfUp;
	static int detectMotionMode = 0;

	static auto start = std::chrono::system_clock::now();
	static auto end = std::chrono::system_clock::now();
	static auto temp = std::chrono::system_clock::now();

	switch (detectMotionMode) {
		case 0:
		for(int i = a.x; i < b.x; i++){
			uchar j = msk_img.at<uchar>(b.y,i);
			if(j == 255) posXOfWhiteOfBottom.push_back(i);
		}
		if(posXOfWhiteOfBottom.size() > 10){
			start = std::chrono::system_clock::now();

			for(auto p: posXOfWhiteOfBottom){
				bottomPos.x += p;
			}
			bottomPos.x /= (int) posXOfWhiteOfBottom.size();
			detectMotionMode = 1;//下が検出されたので上の検出に移る
		}
		break;
		case 1:
		temp = std::chrono::system_clock::now();
		if(std::chrono::duration_cast<std::chrono::milliseconds>(temp-start).count() < 2000){//一定時間上が検出されるのを待つ
			// std::cout <<  std::chrono::duration_cast<std::chrono::milliseconds>(temp-start).count() << std::endl;
			for(int i = a.x; i < b.x; i++){
				uchar j = msk_img.at<uchar>(a.y, i);
				if(j == 255) posXOfWhiteOfUp.push_back(i);
			}
			if(posXOfWhiteOfUp.size() > 10){
				end = std::chrono::system_clock::now();
				for(auto p: posXOfWhiteOfUp){	
					upPos.x += p;
				}
				upPos.x /= (int) posXOfWhiteOfUp.size();
				throwDirection[0] = upPos.x - bottomPos.x;
				throwDirection[1] = 0;
				throwDirection[2] = upPos.y - bottomPos.y;//yが負ならzの奥方向を向く
				throwForce = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
				pthread_mutex_lock( &mCVmode);;
				CVmode = 3;
				pthread_mutex_unlock( &mCVmode);
				detectMotionMode = 0;
				init = false;
			} else{
				detectMotionMode = 1;//ここでループさせるようにする
			}
		}else {
			throwDirection[0] = 0;
			throwDirection[1] = 0;
			throwDirection[2] = upPos.y - bottomPos.y;
			throwForce = 0;
			bottomPos = cv::Point(0, b.y);
			upPos = cv::Point(0, b.y);
			detectMotionMode = 0;//検出ミスなのでまた最初に戻す
			printf("RETRY!!!");
		}
		// std::cout << upPos.x << ", " << bottomPos.x << ", " <<throwDirection[0] <<  ", "<< throwDirection[1] << ", " << throwDirection[2] << "\t" << throwForce << std::endl;
		break;
	}

	cv::rectangle(frame, {350, 80}, {640, 400}, cv::Scalar(255), 5, 8, 0);
	cv::rectangle(msk_img, {350, 80}, {640, 400}, cv::Scalar(255), 5, 8, 0);
	cv::line(frame, {160, 0}, {160, 480}, cv::Scalar(255,0,0), 5, 8, 0);
	cv::imshow("input", frame);

	cv::imshow("mask", msk_img);
	return;
}