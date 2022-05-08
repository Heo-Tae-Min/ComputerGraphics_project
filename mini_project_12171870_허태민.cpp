#define _CRT_SECURE_NO_WARNINGS 
#include <gl/glut.h>
#include <gl/freeglut.h>
#include <cmath>
#include <stdio.h>
#include "ObjParser.h"
#include "bmpfuncs.h"
#include "GL/glext.h"
#include <mmsystem.h>    
#pragma comment(lib,"winmm.lib") 

#define m_pi 3.14159265358979

// ��ƿ �Լ��� =================================================================================
void normalize(float arr[]) {
	float size = sqrt(pow(arr[0], 2) + pow(arr[1], 2) + pow(arr[2], 2));
	for (int i = 0; i < 3; i++) {
		arr[i] = arr[i] / size;
	}
}
float* cross(float arr1[], float arr2[]) {
	float arr3[3];
	arr3[0] = arr1[1] * arr2[2] - arr1[2] * arr2[1];
	arr3[1] = arr1[2] * arr2[0] - arr1[0] * arr2[2];
	arr3[2] = arr1[0] * arr2[1] - arr1[1] * arr2[0];

	return arr3;
}
// �������� part ===============================================================================================
// ���콺 ���� ���� ������
int mouse_x = 0;
int mouse_y = 0;
float x_offset;
float y_offset;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float camPos[3] = { 0.0f, 1.0f, 3.0f }; // ī�޶� ��ġ
float camDir[3] = { 0.0f, 0.0f, -1.0f };
float camUp[3] = { 0.0f, 1.0f, 0.0f };
float camTarget[3]; // ������ ���

// ĳ���� �̵� ���� ������
int w_pressed = 0;
int s_pressed = 0;
int a_pressed = 0;
int d_pressed = 0;

// ���� ���� ������
int goal = 0;
int score = 0;
int shoot = 0;
int combo = 0;
int playing = 0;
int bounce = 0;
int x_bound = 0;
int z_bound = 0;
int map_change = 0;
int backboard_col = 0;
float ball_spin = 0;
float ball_size = 0.2;
float gravity = 0.001;
float ball_speed = 0.09;
float ball_dir[3];
float ball_location[3];

// map ���� ������
float map_x = 8;
float map_y = 8;
float map_z = 20;

// ���� ����
int point = 1;

// ���� �ð� ���� ����
double game_time = 240;
double rest_time;

// hand ���� ����
int hand_count = 0;
char handobj[50];

// ��Ƽ �ٸ���� ���� ����
bool antialiase_on = false;

// ������Ʈ �� �ؽ�ó ���� ����
ObjParser* ball;
ObjParser* hoop;
ObjParser* hand;
ObjParser* goal_3d;
GLuint ball_tex;
GLuint floor_tex;
GLuint hoop_tex;
GLuint map_tex[6];

// �ݹ� �Լ��� ===================================================================================================
// passive mouse motion�� callback �Լ�, passive�� ���콺�� ��ġ ���� ����
// ���� ��ǥ�� ���� ��ǥ�� ���̸� �̿��Ͽ� ������ ����
void mouse_motion(int x, int y) {
	if (point == 1) {
		if (mouse_x == 0 && mouse_y == 0) {
			mouse_x = x;
			mouse_y = y;
		}
		else {
			x_offset = x - mouse_x;
			y_offset = mouse_y - y;
			mouse_x = x;
			mouse_y = y;

			float sensitivity = 0.3;
			x_offset *= sensitivity;
			y_offset *= sensitivity;

			yaw += x_offset;
			pitch += y_offset;

			if (pitch > 89.0f)
				pitch = 89.0;
			if (pitch < -89.0f)
				pitch = -89.0;

			camDir[0] = cos(m_pi / 180 * yaw) * cos(m_pi / 180 * pitch);
			camDir[1] = sin(m_pi / 180 * pitch);
			camDir[2] = sin(m_pi / 180 * yaw) * cos(m_pi / 180 * pitch);
			normalize(camDir);
			glutPostRedisplay();
		}
	}
}

// ���콺 Ŭ���� ���� ���� ����
void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
		if (shoot != 1) {
			shoot = 1;
			ball_dir[0] = ball_speed * camDir[0];
			ball_dir[1] = ball_speed * camDir[1];
			ball_dir[2] = ball_speed * camDir[2];
			ball_location[0] = camPos[0];
			ball_location[1] = camPos[1];
			ball_location[2] = camPos[2];
		}
	}
}

// ���콺 ���� ���� ���� ���� ����
void mousewheel(int wheel, int direction, int x, int y) {
	if (direction > 0)
	{
		if (ball_speed < 0.2) {
			ball_speed += 0.01;
			printf("Ball Speed : %f\n", ball_speed);
		}
	}
	else
	{
		if (ball_speed > 0) {
			ball_speed -= 0.01;
			printf("Ball Speed : %f\n", ball_speed);
		}
	}
}

void idle(void) {
	if (shoot) {
		ball_spin += 10;
		if (ball_spin > 360)
			ball_spin -= 360;
	}

	if (glutGet(GLUT_ELAPSED_TIME) / 1000.0 > game_time)
		rest_time = 0;
	else
		rest_time = game_time - glutGet(GLUT_ELAPSED_TIME) / 1000.0;

	// �� �ִϸ��̼� ���� �ڵ�
	// �� �ִϸ��̼��� �߰��ϸ� ������ ��������� ��û�� ���� �ɸ��ϴ�..
	/*if (hand_count == 20)
		hand_count = 0;
	else
		hand_count++;

	sprintf(handobj, "hand/hand_0000%d.obj", hand_count);
	handobj[strlen(handobj)] = 0;

	*hand = ObjParser(handobj);
	glutPostRedisplay();*/
}

void resize(int width, int height) {
	glViewport(0, 0, width, height); // Viewport T.F
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (float)width / (float)height, 0.1, 500); // Projection T.F
	glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {

	// 3��Ī ����
	if (key == '3') {
		printf("���� ���� : 3��Ī\n");
		point = 3;
		ball_dir[1] = 1;
	}
	// 1��Ī ����
	if (key == '1') {
		printf("���� ���� : 1��Ī\n");
		point = 1;
		printf("%d\n", point);
	}

	// ĳ���� �̵� ����
	if (key == 'w') w_pressed = 1;
	if (key == 's') s_pressed = 1;
	if (key == 'a') a_pressed = 1;
	if (key == 'd') d_pressed = 1;

	// �� ����
	if (key == 'r') {
		shoot = 0;
		combo = 0;
		x_bound = 0;
		z_bound = 0;
		backboard_col = 0;
		printf("���� ���� �Ǿ����ϴ�.\n");
	}

	// blending ��� 
	if (key == 'p') {
		if (antialiase_on) {
			printf("Antialiasing ����� �������ϴ�.\n");
			antialiase_on = false;
			glDisable(GL_BLEND);
		}
		else {
			printf("Antialiasing ����� �������ϴ�.\n");
			antialiase_on = true;
			glEnable(GL_BLEND);
		}
	}

	glutPostRedisplay();
}

void keyup(unsigned char key, int x, int y) {
	if (key == 'w') w_pressed = 0;
	if (key == 's') s_pressed = 0;
	if (key == 'a') a_pressed = 0;
	if (key == 'd') d_pressed = 0;
}

// ĳ���� �̵� ==================================================================================================
// �� Ű�� ���� ��츦 ���� �ٷ�, ���� Ű�� ������ ��쵵 �ٷ�
void ch_move() {
	float camSpeed = 0.07f;
	float* cross_arr;
	if (w_pressed) {
		if (point == 1) {
			camPos[0] += camSpeed * camDir[0];
			camPos[2] += camSpeed * camDir[2];
		}
		else if (point == 3)
			camPos[2] -= camSpeed;
		glutPostRedisplay();
	}
	if (s_pressed) {
		if (point == 1) {
			camPos[0] -= camSpeed * camDir[0];
			camPos[2] -= camSpeed * camDir[2];
		}
		else if (point == 3)
			camPos[2] += camSpeed;
		glutPostRedisplay();
	}
	if (a_pressed) {
		if (point == 1) {
			cross_arr = cross(camDir, camUp);
			normalize(cross_arr);
			camPos[0] -= camSpeed * cross_arr[0];
			camPos[2] -= camSpeed * cross_arr[2];
		}
		else if (point == 3)
			camPos[0] -= camSpeed;
		glutPostRedisplay();
	}
	if (d_pressed) {
		if (point == 1) {
			cross_arr = cross(camDir, camUp);
			normalize(cross_arr);
			camPos[0] += camSpeed * cross_arr[0];
			camPos[2] += camSpeed * cross_arr[2];
		}
		else if (point == 3)
			camPos[0] += camSpeed;
		glutPostRedisplay();
	}


	// ĳ���Ϳ� ���� �浹ó��
	if (camPos[0] <= -1 * map_x) camPos[0] = -1 * map_x;
	else if (camPos[0] >= map_x) camPos[0] = map_x;
	if (camPos[2] <= 0) camPos[2] = 0;
	else if (camPos[2] >= map_z) camPos[2] = map_z;

}

// �ؽ�ó �Լ��� =================================================================================================

void ball_texture() {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("./img/ball.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &ball_tex);
	glBindTexture(GL_TEXTURE_2D, ball_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void hoop_texture() {
	int imgWidth, imgHeight, channels;
	uchar* img = readImageData("./img/hoop.bmp", &imgWidth, &imgHeight, &channels);

	int texNum = 1;
	glGenTextures(texNum, &hoop_tex);
	glBindTexture(GL_TEXTURE_2D, hoop_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, img);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

void map_texture() {
	int w, h, ch;
	glGenTextures(6, map_tex); // n���� ������� �ʰ� �ִ� texture name�� ����
	for (int i = 0; i < 6; i++) {
		glBindTexture(GL_TEXTURE_2D, map_tex[i]);

		// bmp���� �̸��� ��� string ����
		char buf[50];
		if(combo < 5)
			sprintf(buf, "./img/environment/MapImage%d.bmp", i);
		else
			sprintf(buf, "./img/environment/FinalMap%d.bmp", i);

		buf[strlen(buf)] = 0;

		uchar* img = readImageData(buf, &w, &h, &ch); // �̹��� �ҷ�����
		glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}
}

// ===========================================================================================================

void init(void) {
	glClearColor(1, 1, 1, 1);

	// Antialiasing ���� �ڵ�
	// Ű���� callback �Լ����� glEnable, glDisable GL_BLENDER ����
	// ��Ƽ �ٸ������ ����ϸ� �����￡ �缱�� �߻��ϴ� ������ �ֽ��ϴ�..
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// Depth Buffer & Test ���� �ڵ�
	glClearDepth(1.0f);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);

	// ������ ������ ���� �迭 
	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	// ���� ON
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Material
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);

	// Texture Mapping ���� �ڵ�
	glEnable(GL_TEXTURE_2D);
	ball_texture();
	hoop_texture();
	map_texture();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Draw �Լ��� ================================================================================================

void draw_map() {
	glDisable(GL_LIGHTING);
	glColor4f(1, 1, 1, 0.75);
	// px
	glBindTexture(GL_TEXTURE_2D, map_tex[0]);
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(1, 0); glVertex3i(map_x, 0, 0);
	glTexCoord2f(0, 0); glVertex3i(map_x, 0, map_z);
	glTexCoord2f(0, 1); glVertex3i(map_x, map_y, map_z);
	glTexCoord2f(1, 1); glVertex3i(map_x, map_y, 0);
	glEnd();

	// nx
	glBindTexture(GL_TEXTURE_2D, map_tex[1]);
	glBegin(GL_QUADS);
	glNormal3f(1, 0, 0);
	glTexCoord2f(0, 0); glVertex3i(-1 * map_x, 0, 0);
	glTexCoord2f(1, 0); glVertex3i(-1 * map_x, 0, map_z);
	glTexCoord2f(1, 1); glVertex3i(-1 * map_x, map_y, map_z);
	glTexCoord2f(0, 1); glVertex3i(-1 * map_x, map_y, 0);
	glEnd();

	// py
	glBindTexture(GL_TEXTURE_2D, map_tex[2]);
	glBegin(GL_QUADS);
	glNormal3f(0, -1, 0);
	glTexCoord2f(0, 0); glVertex3i(-1 * map_x, map_y, map_z);
	glTexCoord2f(0, 1); glVertex3i(-1 * map_x, map_y, 0);
	glTexCoord2f(1, 1); glVertex3i(map_x, map_y, 0);
	glTexCoord2f(1, 0); glVertex3i(map_x, map_y, map_z);
	glEnd();

	// ny
	glBindTexture(GL_TEXTURE_2D, map_tex[3]);
	glBegin(GL_QUADS);
	glNormal3f(0, 1, 0);
	glTexCoord2f(1, 0); glVertex3i(-1 * map_x, 0, map_z);
	glTexCoord2f(1, 1); glVertex3i(-1 * map_x, 0, 0);
	glTexCoord2f(0, 1); glVertex3i(map_x, 0, 0);
	glTexCoord2f(0, 0); glVertex3i(map_x, 0, map_z);
	glEnd();

	// pz
	glBindTexture(GL_TEXTURE_2D, map_tex[4]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, -1);
	glTexCoord2f(0, 0); glVertex3i(-1 * map_x, 0, map_z);
	glTexCoord2f(0, 1); glVertex3i(-1 * map_x, map_y, map_z);
	glTexCoord2f(1, 1); glVertex3i(map_x, map_y, map_z);
	glTexCoord2f(1, 0); glVertex3i(map_x, 0, map_z);
	glEnd();

	// nz
	glBindTexture(GL_TEXTURE_2D, map_tex[5]);
	glBegin(GL_QUADS);
	glNormal3f(0, 0, 1);
	glTexCoord2f(1, 0); glVertex3i(-1 * map_x, 0, 0);
	glTexCoord2f(1, 1); glVertex3i(-1 * map_x, map_y, 0);
	glTexCoord2f(0, 1); glVertex3i(map_x, map_y, 0);
	glTexCoord2f(0, 0); glVertex3i(map_x, 0, 0);
	glEnd();

	glEnable(GL_LIGHTING);
}

void draw_object(ObjParser* objParser)
{
	glColor4f(1, 1, 1, 1);
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
}

void draw_object_and_texture(ObjParser* objParser, GLuint texture)
{
	glColor4f(1, 1, 1, 0.75);
	glEnable(GL_TEXTURE_2D);	// texture �� ������ ���� enable
	glBindTexture(GL_TEXTURE_2D, texture);
	glBegin(GL_TRIANGLES);
	for (unsigned int n = 0; n < objParser->getFaceSize(); n += 3) {
		glTexCoord2f(objParser->textures[objParser->textureIdx[n] - 1].x,
			objParser->textures[objParser->textureIdx[n] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n] - 1].x,
			objParser->normal[objParser->normalIdx[n] - 1].y,
			objParser->normal[objParser->normalIdx[n] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n] - 1].x,
			objParser->vertices[objParser->vertexIdx[n] - 1].y,
			objParser->vertices[objParser->vertexIdx[n] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 1] - 1].x,
			objParser->textures[objParser->textureIdx[n + 1] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 1] - 1].x,
			objParser->normal[objParser->normalIdx[n + 1] - 1].y,
			objParser->normal[objParser->normalIdx[n + 1] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 1] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 1] - 1].z);

		glTexCoord2f(objParser->textures[objParser->textureIdx[n + 2] - 1].x,
			objParser->textures[objParser->textureIdx[n + 2] - 1].y);
		glNormal3f(objParser->normal[objParser->normalIdx[n + 2] - 1].x,
			objParser->normal[objParser->normalIdx[n + 2] - 1].y,
			objParser->normal[objParser->normalIdx[n + 2] - 1].z);
		glVertex3f(objParser->vertices[objParser->vertexIdx[n + 2] - 1].x,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].y,
			objParser->vertices[objParser->vertexIdx[n + 2] - 1].z);
	}
	glEnd();
}

// ���� ���� �Լ�
void shooting() {
	glPushMatrix();
	glColor4f(1, 1, 1, 1);

	// �Լ��� �ѹ� ȣ��� ������, direction�� ������ ���� ��ġ���� ���� ���ݾ� �̵���
	for (int i = 0; i < 3; i++) {
		ball_location[i] += ball_dir[i];
	}
	// �����̵� ��ȯ
	glTranslatef(ball_location[0], ball_location[1], ball_location[2]);
	// ��Ǽ��� ���� ���� ȸ�� ����
	glRotatef(ball_spin, 1, 0, ball_dir[0] / ball_dir[2]);
	draw_object_and_texture(ball, ball_tex);
	glPopMatrix();

	// ������ y ���п� �߷��� ���������� ���� �������� �׸��� ��
	ball_dir[1] -= gravity;

	// �� �浹, ���� �浹�ϸ� ���� �ٱ� ������, 0.7�� ��
	// ���� �ε��� ������ �´� ������ �ݴ�������� �ٲپ� �� �ݻ縦 ����
	// sound �߰�
	if (ball_location[0] <= -1 * (map_x - ball_size) || ball_location[0] >= map_x - ball_size) {
		if (x_bound == 0) {
			ball_dir[0] *= -0.7;
			x_bound = 1;
			if(playing == 0 && map_change != 1) PlaySound(TEXT("./sound/ball.wav"), NULL, SND_ASYNC | SND_ALIAS);
		}
	}
	if (ball_location[1] <= ball_size) {
		ball_dir[1] *= -0.7;
		bounce++;
		if (playing == 0 && map_change != 1) PlaySound(TEXT("./sound/ball.wav"), NULL, SND_ASYNC | SND_ALIAS);
	}
	if (ball_location[2] <= ball_size || ball_location[2] >= map_z - ball_size) {
		if (z_bound == 0) {
			ball_dir[2] *= -0.7;
			z_bound = 1;
			if (playing == 0 && map_change != 1) PlaySound(TEXT("./sound/ball.wav"), NULL, SND_ASYNC | SND_ALIAS);
		}
	}

	// �麸�� �浹, �󱸰���� �� ���忡 ���� �浹
	if ((-0.77 + ball_size) <= ball_location[0]  && ball_location[0] <= (0.77 - ball_size))
		if ((2.27 + ball_size) <= ball_location[1] && ball_location[1] <= (3.5 - ball_size))
			if (ball_location[2] <= 1 + ball_size)
				if (backboard_col == 0) {
					backboard_col = 1;
					ball_dir[2] *= -0.7;
					if(playing == 0 && map_change != 1) PlaySound(TEXT("./sound/ball.wav"), NULL, SND_ASYNC | SND_ALIAS);
				}

	// �� 
	// ���� x,y,z ��ǥ�� ���� ���� ������ ���� ��, ���� y ��ǥ�� ������ (-)�����̸� ��� ����
	if ((-0.3 + ball_size) <= ball_location[0] && ball_location[0] <= (0.3 - ball_size))
		if ((1.2 + ball_size) <= ball_location[2] && ball_location[2] <= (1.77 - ball_size))
			if ((2.55 - ball_size) <= ball_location[1] && ball_location[1] <= (2.55 + ball_size))
				if (ball_dir[1] < 0 && goal == 0) {
					goal = 1;
				}
	// Score�ø��� ���� �����ϴ� ������ ���� ������ ���ԵǾ� ���� ������ ���� �����Ǵ� ���� ���� ����
	// gool ������ 1�� ���� score�� combe�� up, �׸��� goal�� 0���� �ٲٸ� ���󺹱� �ǹǷ� X
	// goal�� 2�� ���� ��, ���� �� �Ŀ��� �ٴڿ� �� �� �ε����� ���µǵ��� ��
	if (goal == 1) {
		score++;
		combo++;
		goal = 2;
	}
	if (goal == 2) {
		if (bounce == 1) {
			goal = 0;
			shoot = 0;
			playing = 0;
			bounce = 0;
			backboard_col = 0;
		}
	}

	// 5�޺� �޼��� �� �̵��� ���� ���� �� ����
	if (combo == 5 && map_change != 1) {
		map_change = 1;
	}

	// �ٴڿ� �ι� �ε����� ���, �� ���� ���� ���� ��� combo�� ������ ���� ���� reset
	// ���� �̵��Ǿ��� ���¿��ٸ�, ���� ������ �ٽ� �̵�
	if (bounce == 2) {
		if (combo >= 5) {
			combo = 0;
			map_texture();
		}
		shoot = 0;
		combo = 0;
		bounce = 0;
		x_bound = 0;
		z_bound = 0;
		backboard_col = 0;
	}

	glutPostRedisplay();
}

void draw_string(void* font, const char* str, float x_position, float y_position, float red, float green, float blue) {
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(-5, 5, -5, 5); // ȭ���� ��ǥ (-5, -5) ~ (5, 5)

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glColor3f(red, green, blue);
	glRasterPos3f(x_position, y_position, 0);
	for (unsigned int i = 0; i < strlen(str); i++)
		glutBitmapCharacter(font, str[i]);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopAttrib();
}

void draw_text() {
	glViewport(0, 500, 300, 300);
	glLoadIdentity();

	char scoretext[50];
	char combotext[50];
	char timetext[50];
	sprintf(scoretext, "SCORE : %d", score * 100);
	scoretext[strlen(scoretext)] = 0;
	sprintf(timetext, "TIME : %f", rest_time);
	timetext[strlen(timetext)] = 0;
	sprintf(combotext, "COMBO : %d!!", combo);
	combotext[strlen(combotext)] = 0;


	draw_string(GLUT_BITMAP_TIMES_ROMAN_24, scoretext, -4.5, 4, 1, 1, 1);
	draw_string(GLUT_BITMAP_TIMES_ROMAN_24, timetext, -4.5, 3, 1, 1, 1);
	draw_string(GLUT_BITMAP_TIMES_ROMAN_24, combotext, -4.5, 2, 1, 1, 1);
	glutPostRedisplay();
	glFlush();
}

void draw(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (rest_time) {
		// multi viewport�� �̿��� �ؽ�Ʈ ���
		draw_text();

		// viewport  ���� ����
		glViewport(0, 0, 1000, 800);

		// ĳ���� �̵�
		ch_move();

		// �� �ִϸ��̼� ���� �ڵ�
		/*glPushMatrix();
		glTranslatef(-0.3, -0.4, -1);
		draw_object(hand);
		glPopMatrix();*/

		// ���� �־��� ��, ���� ������Ʈ ���
		if (goal) {
			glPushMatrix();
			glColor4f(1, 1, 1, 0.75);
			glTranslatef(0, 0, -4);
			draw_object(goal_3d);
			glPopMatrix();
		}

		// ���� �־��� �� ���� �߰�
		if (goal && playing == 0) {
			PlaySound(TEXT("./sound/bbabba.wav"), NULL, SND_ASYNC | SND_ALIAS);
			playing = 1;
		}

		for (int i = 0; i < 3; i++) {
			camTarget[i] = (camPos[i] + camDir[i]);
		}
		if (point == 1)
			gluLookAt(camPos[0], camPos[1], camPos[2], camTarget[0], camTarget[1], camTarget[2], camUp[0], camUp[1], camUp[2]); // Viewing T.F
		else if (point == 3) {
			gluLookAt(camPos[0], camPos[1] + 6, camPos[2] + 6, camPos[0], camPos[1], camPos[2], 0, 1, 0); // Viewing T.F

			glPushMatrix();
			glTranslatef(camPos[0], camPos[1], camPos[2]);
			glColor4f(1, 1, 1, 1);
			glutSolidSphere(0.1, 20, 20);
			glPopMatrix();
		}

		// ������ ��ġ ����
		GLfloat light_position[] = { 0.0, 10.0, 10.0, 1.0 };
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

		// �麸�� �𼭸� ============================
		glPushMatrix();
		glTranslatef(-0.77, 3.5, 1);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.77, 3.5, 1);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-0.77, 2.27, 1);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0.77, 2.27, 1);
		glutSolidSphere(0.05, 20, 20);
		glPopMatrix();

		// shoot ������ 1�� ���� shooting �Լ� ����, ���ÿ��� ���� X
		if (shoot == 1) shooting();

		// �� ����� map texture�� �� �� ȣ���Ͽ� ���� map �ؽ�ó�� ����
		if (map_change == 1) {
			map_texture();
			map_change = 0;
		}

		draw_map();
		draw_object_and_texture(hoop, hoop_tex);
	}
	else {
		glViewport(0, 0, 1000, 800);
		glLoadIdentity();

		char scoretext[50];
		char combotext[50];
		char timetext[50];
		sprintf(scoretext, "SCORE : %d", score * 100);
		scoretext[strlen(scoretext)] = 0;
		sprintf(timetext, "TIME : %f", rest_time);
		timetext[strlen(timetext)] = 0;
		sprintf(combotext, "COMBO : %d!!", combo);
		combotext[strlen(combotext)] = 0;

		draw_string(GLUT_BITMAP_TIMES_ROMAN_24, scoretext, -1, -0.5, 1, 1, 1);
		draw_string(GLUT_BITMAP_TIMES_ROMAN_24, "GAME OVER", -1, 0.5, 1, 1, 1);
	}
	glFlush();
	glutSwapBuffers();
}

void main_menu_function(int option)
{
	printf("Main menu %d has been selected\n", option);
	if (option == 999) exit(0);
	if (option == 1) init();
}

void func_instruction() {
	printf("��� ��� ==============================================================\n\n");
	printf("W,A,S,D : ĳ���� �̵�\n");
	printf("���콺 Ŀ�� �̵� : ������ �̵�\n");
	printf("���콺 ���� Ŭ�� : �� ������\n");
	printf("���콺 �� UP/DOWN : �� ������ �Ŀ� ����\n");
	printf("Ű���� R : �� ���� (���� : ������ �ϸ� �޺��� ������ϴ�.)\n");
	printf("Ű���� P : ��Ƽ �ٸ���� ON/OFF \n(��Ƽ �ٸ������ ����ϸ�, �����￡ �缱�� ����� ������ �ֽ��ϴ�.)\n");
	printf("======================================================================\n\n");
	printf("���� ���  ==============================================================\n\n");
	printf("���� ���� ��뿡 �ִ� �����Դϴ�. 5�޺� �޼��� ���� �̵��մϴ�.\n");
	printf("======================================================================\n\n");
	printf("���� �Ŀ� 0.8�϶��� ������ ���ο���, 0.9�϶��� 3�� ���ο��� �� ���ϴ�.\n\n");
	
}

int main(int argc, char** argv) {
	// window �ʱ�ȭ
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutInitWindowPosition(800, 100);
	glutCreateWindow("Mini Project 12171870 ���¹�");

	init();
	func_instruction();

	// ���ҽ� �ε�
	ball = new ObjParser("./obj/ball.obj");
	hoop = new ObjParser("./obj/hoop.obj");
	goal_3d = new ObjParser("./obj/goal.obj");

	// �� �ִϸ��̼� ���� �ڵ�
	//hand = new ObjParser("hand/hand00000.obj");

	// Popup menu ���� �� �߰�
	glutCreateMenu(main_menu_function);
	glutAddMenuEntry("Quit", 999);
	glutAddMenuEntry("Init", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Callback �Լ� ����
	glutReshapeFunc(resize);
	glutDisplayFunc(draw);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyup);
	//glutSpecialFunc(specialkey);
	glutPassiveMotionFunc(mouse_motion);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mousewheel);

	// Looping ����
	glutMainLoop();

	return 0;
}