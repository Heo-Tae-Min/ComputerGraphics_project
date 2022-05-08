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

// 유틸 함수들 =================================================================================
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
// 전역변수 part ===============================================================================================
// 마우스 시점 관련 변수들
int mouse_x = 0;
int mouse_y = 0;
float x_offset;
float y_offset;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
float camPos[3] = { 0.0f, 1.0f, 3.0f }; // 카메라 위치
float camDir[3] = { 0.0f, 0.0f, -1.0f };
float camUp[3] = { 0.0f, 1.0f, 0.0f };
float camTarget[3]; // 시점의 대상

// 캐릭터 이동 관련 변수들
int w_pressed = 0;
int s_pressed = 0;
int a_pressed = 0;
int d_pressed = 0;

// 슈팅 관련 변수들
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

// map 관련 변수들
float map_x = 8;
float map_y = 8;
float map_z = 20;

// 시점 변수
int point = 1;

// 실행 시간 관련 변수
double game_time = 240;
double rest_time;

// hand 관련 변수
int hand_count = 0;
char handobj[50];

// 안티 앨리어싱 관련 변수
bool antialiase_on = false;

// 오브젝트 및 텍스처 관련 변수
ObjParser* ball;
ObjParser* hoop;
ObjParser* hand;
ObjParser* goal_3d;
GLuint ball_tex;
GLuint floor_tex;
GLuint hoop_tex;
GLuint map_tex[6];

// 콜백 함수들 ===================================================================================================
// passive mouse motion의 callback 함수, passive한 마우스의 위치 값을 가짐
// 현재 좌표와 직전 좌표의 차이를 이용하여 방향을 구함
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

// 마우스 클릭을 통한 슈팅 구현
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

// 마우스 휠을 통한 슈팅 강도 조절
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

	// 손 애니메이션 관련 코드
	// 손 애니메이션을 추가하면 동작은 실행되지만 엄청난 렉이 걸립니다..
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

	// 3인칭 시점
	if (key == '3') {
		printf("현재 시점 : 3인칭\n");
		point = 3;
		ball_dir[1] = 1;
	}
	// 1인칭 시점
	if (key == '1') {
		printf("현재 시점 : 1인칭\n");
		point = 1;
		printf("%d\n", point);
	}

	// 캐릭터 이동 변수
	if (key == 'w') w_pressed = 1;
	if (key == 's') s_pressed = 1;
	if (key == 'a') a_pressed = 1;
	if (key == 'd') d_pressed = 1;

	// 공 리셋
	if (key == 'r') {
		shoot = 0;
		combo = 0;
		x_bound = 0;
		z_bound = 0;
		backboard_col = 0;
		printf("공이 리셋 되었습니다.\n");
	}

	// blending 기능 
	if (key == 'p') {
		if (antialiase_on) {
			printf("Antialiasing 기능이 켜졌습니다.\n");
			antialiase_on = false;
			glDisable(GL_BLEND);
		}
		else {
			printf("Antialiasing 기능이 꺼졌습니다.\n");
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

// 캐릭터 이동 ==================================================================================================
// 각 키가 눌린 경우를 따로 다뤄, 여러 키가 눌렸을 경우도 다룸
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


	// 캐릭터와 벽의 충돌처리
	if (camPos[0] <= -1 * map_x) camPos[0] = -1 * map_x;
	else if (camPos[0] >= map_x) camPos[0] = map_x;
	if (camPos[2] <= 0) camPos[2] = 0;
	else if (camPos[2] >= map_z) camPos[2] = map_z;

}

// 텍스처 함수들 =================================================================================================

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
	glGenTextures(6, map_tex); // n개의 사용하지 않고 있는 texture name을 생성
	for (int i = 0; i < 6; i++) {
		glBindTexture(GL_TEXTURE_2D, map_tex[i]);

		// bmp파일 이름이 담긴 string 생성
		char buf[50];
		if(combo < 5)
			sprintf(buf, "./img/environment/MapImage%d.bmp", i);
		else
			sprintf(buf, "./img/environment/FinalMap%d.bmp", i);

		buf[strlen(buf)] = 0;

		uchar* img = readImageData(buf, &w, &h, &ch); // 이미지 불러오기
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

	// Antialiasing 관련 코드
	// 키보드 callback 함수에서 glEnable, glDisable GL_BLENDER 수행
	// 안티 앨리어싱을 사용하면 폴리곤에 사선이 발생하는 오류가 있습니다..
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// Depth Buffer & Test 관련 코드
	glClearDepth(1.0f);
	glFrontFace(GL_CW);
	glEnable(GL_DEPTH_TEST);

	// 조명의 설정값 관련 배열 
	GLfloat light_ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	GLfloat light_diffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat light_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	// 조명 ON
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Material
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, 128);

	// Texture Mapping 관련 코드
	glEnable(GL_TEXTURE_2D);
	ball_texture();
	hoop_texture();
	map_texture();

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

// Draw 함수들 ================================================================================================

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
	glEnable(GL_TEXTURE_2D);	// texture 색 보존을 위한 enable
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

// 슈팅 구현 함수
void shooting() {
	glPushMatrix();
	glColor4f(1, 1, 1, 1);

	// 함수가 한번 호출될 때마다, direction의 값들을 현재 위치에서 더해 조금씩 이동함
	for (int i = 0; i < 3; i++) {
		ball_location[i] += ball_dir[i];
	}
	// 평행이동 변환
	glTranslatef(ball_location[0], ball_location[1], ball_location[2]);
	// 사실성을 위해 공의 회전 구현
	glRotatef(ball_spin, 1, 0, ball_dir[0] / ball_dir[2]);
	draw_object_and_texture(ball, ball_tex);
	glPopMatrix();

	// 방향의 y 성분에 중력을 점차적으로 더해 포물선을 그리게 함
	ball_dir[1] -= gravity;

	// 벽 충돌, 벽에 충돌하면 힘이 줄기 때문에, 0.7배 함
	// 벽에 부딪힌 방향의 맞는 성분을 반대방향으로 바꾸어 벽 반사를 구현
	// sound 추가
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

	// 백보드 충돌, 농구골대의 백 보드에 대한 충돌
	if ((-0.77 + ball_size) <= ball_location[0]  && ball_location[0] <= (0.77 - ball_size))
		if ((2.27 + ball_size) <= ball_location[1] && ball_location[1] <= (3.5 - ball_size))
			if (ball_location[2] <= 1 + ball_size)
				if (backboard_col == 0) {
					backboard_col = 1;
					ball_dir[2] *= -0.7;
					if(playing == 0 && map_change != 1) PlaySound(TEXT("./sound/ball.wav"), NULL, SND_ASYNC | SND_ALIAS);
				}

	// 골 
	// 공의 x,y,z 좌표가 링의 안쪽 영역에 있을 때, 현재 y 좌표의 방향이 (-)방향이면 골로 인정
	if ((-0.3 + ball_size) <= ball_location[0] && ball_location[0] <= (0.3 - ball_size))
		if ((1.2 + ball_size) <= ball_location[2] && ball_location[2] <= (1.77 - ball_size))
			if ((2.55 - ball_size) <= ball_location[1] && ball_location[1] <= (2.55 + ball_size))
				if (ball_dir[1] < 0 && goal == 0) {
					goal = 1;
				}
	// Score올리기 골을 인정하는 영역에 공이 여러번 포함되어 여러 번으로 복수 인정되는 것을 막기 위함
	// gool 변수가 1일 때는 score와 combe를 up, 그리고 goal을 0으로 바꾸면 원상복구 되므로 X
	// goal을 2로 변경 후, 골이 들어간 후에는 바닥에 한 번 부딪히면 리셋되도록 함
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

	// 5콤보 달성시 맵 이동을 위한 변수 값 변경
	if (combo == 5 && map_change != 1) {
		map_change = 1;
	}

	// 바닥에 두번 부딪히는 경우, 즉 골이 들어가지 않은 경우 combo를 포함한 여러 변수 reset
	// 맵이 이동되었던 상태였다면, 이전 맵으로 다시 이동
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
	gluOrtho2D(-5, 5, -5, 5); // 화면의 좌표 (-5, -5) ~ (5, 5)

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
		// multi viewport를 이용한 텍스트 출력
		draw_text();

		// viewport  범위 설정
		glViewport(0, 0, 1000, 800);

		// 캐릭터 이동
		ch_move();

		// 손 애니메이션 관련 코드
		/*glPushMatrix();
		glTranslatef(-0.3, -0.4, -1);
		draw_object(hand);
		glPopMatrix();*/

		// 골을 넣었을 때, 글자 오브젝트 출력
		if (goal) {
			glPushMatrix();
			glColor4f(1, 1, 1, 0.75);
			glTranslatef(0, 0, -4);
			draw_object(goal_3d);
			glPopMatrix();
		}

		// 골을 넣었을 때 사운드 추가
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

		// 조명의 위치 설정
		GLfloat light_position[] = { 0.0, 10.0, 10.0, 1.0 };
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);

		// 백보드 모서리 ============================
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

		// shoot 변수가 1일 때만 shooting 함수 동작, 평상시에는 동작 X
		if (shoot == 1) shooting();

		// 맵 변경시 map texture를 한 번 호출하여 변한 map 텍스처를 적용
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
	printf("기능 목록 ==============================================================\n\n");
	printf("W,A,S,D : 캐릭터 이동\n");
	printf("마우스 커서 이동 : 조준점 이동\n");
	printf("마우스 왼쪽 클릭 : 공 던지기\n");
	printf("마우스 휠 UP/DOWN : 공 던지는 파워 조절\n");
	printf("키보드 R : 공 리셋 (주의 : 리셋을 하면 콤보는 사라집니다.)\n");
	printf("키보드 P : 안티 앨리어싱 ON/OFF \n(안티 앨리어싱을 사용하면, 폴리곤에 사선이 생기는 문제가 있습니다.)\n");
	printf("======================================================================\n\n");
	printf("게임 방법  ==============================================================\n\n");
	printf("공을 던져 골대에 넣는 게임입니다. 5콤보 달성시 맵을 이동합니다.\n");
	printf("======================================================================\n\n");
	printf("슈팅 파워 0.8일때는 자유투 라인에서, 0.9일때는 3점 라인에서 잘 들어갑니다.\n\n");
	
}

int main(int argc, char** argv) {
	// window 초기화
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutInitWindowPosition(800, 100);
	glutCreateWindow("Mini Project 12171870 허태민");

	init();
	func_instruction();

	// 리소스 로드
	ball = new ObjParser("./obj/ball.obj");
	hoop = new ObjParser("./obj/hoop.obj");
	goal_3d = new ObjParser("./obj/goal.obj");

	// 손 애니메이션 관련 코드
	//hand = new ObjParser("hand/hand00000.obj");

	// Popup menu 생성 및 추가
	glutCreateMenu(main_menu_function);
	glutAddMenuEntry("Quit", 999);
	glutAddMenuEntry("Init", 1);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	// Callback 함수 정의
	glutReshapeFunc(resize);
	glutDisplayFunc(draw);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyup);
	//glutSpecialFunc(specialkey);
	glutPassiveMotionFunc(mouse_motion);
	glutMouseFunc(mouse);
	glutMouseWheelFunc(mousewheel);

	// Looping 시작
	glutMainLoop();

	return 0;
}