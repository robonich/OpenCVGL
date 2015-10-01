#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define WINDOW_X (500)
#define WINDOW_Y (500)
#define WINDOW_NAME "test1"

void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void specialkeydown(int key, int x, int y);

void draw_square1();
void draw_square2();
void draw_square3();
void draw_hexagon();
void draw_nxagon(int n);



// グローバル変数
int g_display_mode = 1;

int nxagon = 3;

int main(int argc, char *argv[]){
	/* OpenGLの初期化 */
	init_GL(argc,argv);

	/* このプログラム特有の初期化 */
	init();

	/* コールバック関数の登録 */
	set_callback_functions();

	/* メインループ */
	glutMainLoop();  
    // 無限ループ。コールバック関数が呼ばれるまでずっと実行される。

	return 0;
}

void init_GL(int argc, char *argv[]){
	glutInit(&argc, argv);                   // OpenGLの初期化
	glutInitDisplayMode(GLUT_RGBA);           // ディスプレイモードをRGBAモードに設定
	glutInitWindowSize(WINDOW_X, WINDOW_Y);    // ウィンドウサイズを指定
	glutCreateWindow(WINDOW_NAME);            // ウィンドウを「生成」。まだ「表示」はされない。
}

void init(){
	glClearColor(0.0, 0.0, 0.0, 0.0);         // 背景の塗りつぶし色を指定
}

void set_callback_functions(){
	glutDisplayFunc(glut_display);            // ディスプレイに変化があった時に呼ばれるコールバック関数を登録
	glutKeyboardFunc(glut_keyboard);          // キーボードに変化があった時に呼び出されるコールバック関数を登録
	glutSpecialFunc(specialkeydown);          //矢印キーなどの特殊入力を検知する
}

void specialkeydown(int key, int x, int y)
{
     if( key == GLUT_KEY_UP )//矢印「上」
     {
		nxagon++;
     }

     if( key == GLUT_KEY_DOWN )//矢印「下」
     {
		if(nxagon > 3)
			nxagon--;
     }

     if( key == GLUT_KEY_LEFT )
     {
     }

     if( key == GLUT_KEY_RIGHT )
     {
     }
     glutPostRedisplay();
}

// キーボードに変化があった時に呼び出されるコールバック関数。
void glut_keyboard(unsigned char key, int x, int y){
	switch(key){

	case 'q':
	case 'Q':
	case '\033': // Escキーのこと
		exit(0);
	case '1':
		g_display_mode = 1;
		break;
	case '2':
		g_display_mode = 2;
		break;
	case '3':
		g_display_mode = 3;
		break;
	case '4':
	    g_display_mode = 4;
	    break;
	case 'n':
		g_display_mode = 5;
		break;
	}

	glutPostRedisplay(); // 「ディスプレイのコールバック関数を呼んで」と指示する。
}

// ディスプレイに変化があった時に呼び出されるコールバック関数。
// 「ディスプレイに変化があった時」は、glutPostRedisplay() で指示する。
void glut_display(){
	glClear(GL_COLOR_BUFFER_BIT); // 今まで画面に描かれていたものを消す

	switch(g_display_mode){
	case 1:
		draw_square1();
		break;
	case 2:
		draw_square2();
		break;
	case 3:
		draw_square3();
		break;
	case 4:
		draw_hexagon();
		break;
	case 5:
		draw_nxagon(nxagon);
		break;
	}

	glFlush(); // ここで画面に描画をする
}

void draw_square1(){
	glBegin(GL_LINE_LOOP);

	glColor3d(1.0, 0.0, 0.0);
	glVertex2d(-0.9, -0.9);
	glVertex2d(0.9, -0.9);
	glVertex2d(0.9, 0.9);
	glVertex2d(-0.9, 0.9);

	glEnd();
}

void draw_square2(){
	glBegin(GL_POLYGON);

	glColor3d(1.0, 0.0, 0.0);
	glVertex2d(-0.9, -0.9);
	glVertex2d(0.9, -0.9);
	glVertex2d(0.9, 0.9);
	glVertex2d(-0.9, 0.9);

	glEnd();
}

void draw_square3(){
	glBegin(GL_POLYGON);

	glColor3d(1.0, 0.0, 0.0);
	glVertex2d(-0.9, -0.9);
	glColor3d(1.0, 1.0, 0.0);
	glVertex2d(0.9, -0.9);
	glColor3d(0.0, 1.0, 1.0);
	glVertex2d(0.9, 0.9);
	glColor3d(0.0, 0.0, 0.0);
	glVertex2d(-0.9, 0.9);

	glEnd();
}

void draw_hexagon(){
	glBegin(GL_LINE_LOOP);

	glColor3d(0.0, 1.0, 0.0);

	glVertex2d(0.9, 0);
	glVertex2d(0.45, 0.779);
	glVertex2d(-0.45, 0.779);
	glVertex2d(-0.9, 0);
	glVertex2d(-0.45, -0.779);
	glVertex2d(0.45, -0.779);

	glEnd();
}

void draw_nxagon(int n){
	glBegin(GL_LINE_LOOP);

	glColor3d(0.0, 0.0, 1.0);

	double theta = 2 * M_PI / (double)n;

	// double rotation[2][2] = {
	// 	{cos(theta), -sin(theta)},
	// 	{sin(theta), cos(theta)}
	// }

	double prevPoint[2], point[2] = {0, 0.9};

	for(int i = 0; i < n; i++){
		glVertex2dv(point);
		prevPoint[0] = point[0];
		prevPoint[1] = point[1];
		point[0] = prevPoint[0] * cos(theta) - prevPoint[1] * sin(theta);
		point[1] = prevPoint[0] * sin(theta) + prevPoint[1] * cos(theta);
	}
	glEnd();
}