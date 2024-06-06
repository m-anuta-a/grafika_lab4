#include "Render.h"




#include <windows.h>

#include <GL\gl.h>
#include <GL\glu.h>
#include "GL\glext.h"

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "MyShaders.h"

#include "ObjLoader.h"
#include "GUItextRectangle.h"

#include "Texture.h"

GuiTextRectangle rec;

bool textureMode = true;
bool lightMode = true;


//��������� ������ ��� ��������� ����
#define POP glPopMatrix()
#define PUSH glPushMatrix()


ObjFile* model;

Texture texture1;
Texture sTex;
Texture rTex;
Texture tBox;

Shader s[10];  //��������� ��� ������ ��������
Shader frac;
Shader cassini;




//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;


	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	virtual void SetUpCamera()
	{

		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ���������!
class WASDcamera :public CustomCamera
{
public:

	float camSpeed;

	WASDcamera()
	{
		camSpeed = 0.4;
		pos.setCoords(5, 5, 5);
		lookPoint.setCoords(0, 0, 0);
		normal.setCoords(0, 0, 1);
	}

	virtual void SetUpCamera()
	{

		if (OpenGL::isKeyPressed('W'))
		{
			Vector3 forward = (lookPoint - pos).normolize() * camSpeed;
			pos = pos + forward;
			lookPoint = lookPoint + forward;

		}
		if (OpenGL::isKeyPressed('S'))
		{
			Vector3 forward = (lookPoint - pos).normolize() * (-camSpeed);
			pos = pos + forward;
			lookPoint = lookPoint + forward;

		}

		LookAt();
	}

} WASDcam;


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}


	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		Shader::DontUseShaders();
		bool f1 = glIsEnabled(GL_LIGHTING);
		glDisable(GL_LIGHTING);
		bool f2 = glIsEnabled(GL_TEXTURE_2D);
		glDisable(GL_TEXTURE_2D);
		bool f3 = glIsEnabled(GL_DEPTH_TEST);

		glDisable(GL_DEPTH_TEST);
		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}
		/*
		if (f1)
			glEnable(GL_LIGHTING);
		if (f2)
			glEnable(GL_TEXTURE_2D);
		if (f3)
			glEnable(GL_DEPTH_TEST);
			*/
	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����



//������ ���������� ����
int mouseX = 0, mouseY = 0;




float offsetX = 0, offsetY = 0;
float zoom = 1;
float Time = 0;
int tick_o = 0;
int tick_n = 0;

//���������� �������� ����
void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	if (OpenGL::isKeyPressed(VK_LBUTTON))
	{
		offsetX -= 1.0 * dx / ogl->getWidth() / zoom;
		offsetY += 1.0 * dy / ogl->getHeight() / zoom;
	}



	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y, 60, ogl->aspect);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

//���������� �������� ������  ����
void mouseWheelEvent(OpenGL* ogl, int delta)
{


	float _tmpZ = delta * 0.003;
	if (ogl->isKeyPressed('Z'))
		_tmpZ *= 10;
	zoom += 0.2 * zoom * _tmpZ;


	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;
}

//���������� ������� ������ ����������
bool IsDuck = true;
void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'S')
	{
		frac.LoadShaderFromFile();
		frac.Compile();

		s[0].LoadShaderFromFile();
		s[0].Compile();

		cassini.LoadShaderFromFile();
		cassini.Compile();
	}

	if (key == 'N') {
		if (IsDuck) {
			IsDuck = false;
		}
		else {
			IsDuck = true;
		}
	}

	if (key == 'Q') {
		Time = 0;
	}
		
}

void keyUpEvent(OpenGL* ogl, int key)
{

}


//void DrawQuad()
//{
//	double A[] = { 0,0 };
//	double B[] = { 1,0 };
//	double C[] = { 1,1 };
//	double D[] = { 0,1 };
//	glBegin(GL_QUADS);
//	glColor3d(.5, 0, 0);
//	glNormal3d(0, 0, 1);
//	glTexCoord2d(0, 0);
//	glVertex2dv(A);
//	glTexCoord2d(1, 0);
//	glVertex2dv(B);
//	glTexCoord2d(1, 1);
//	glVertex2dv(C);
//	glTexCoord2d(0, 1);
//	glVertex2dv(D);
//	glEnd();
//}

GLuint texId;

ObjFile objModel, monkey, duck, dada1, papa;

Texture monkeyTex, duckTex;

//����������� ����� ������ ��������
void initRender(OpenGL* ogl)
{

	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);




	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	//ogl->mainCamera = &WASDcam;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);


	texture1.loadTextureFromFile("textures\\fff.bmp");   //�������� �������� �� �����


	frac.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	frac.FshaderFileName = "shaders\\frac.frag"; //��� ����� ������������ �������
	frac.LoadShaderFromFile(); //��������� ������� �� �����
	frac.Compile(); //�����������

	cassini.VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	cassini.FshaderFileName = "shaders\\cassini.frag"; //��� ����� ������������ �������
	cassini.LoadShaderFromFile(); //��������� ������� �� �����
	cassini.Compile(); //�����������


	s[0].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[0].FshaderFileName = "shaders\\light.frag"; //��� ����� ������������ �������
	s[0].LoadShaderFromFile(); //��������� ������� �� �����
	s[0].Compile(); //�����������

	s[1].VshaderFileName = "shaders\\v.vert"; //��� ����� ���������� ������
	s[1].FshaderFileName = "shaders\\textureShader.frag"; //��� ����� ������������ �������
	s[1].LoadShaderFromFile(); //��������� ������� �� �����
	s[1].Compile(); //�����������



	/* ��� ��� ��� ������� ������ *.obj �����, ��� ��� ��� ��������� �� ���������� � ���������� �������,
	  ������������ �� ����� ����������, � ������������ ������ � *.obj_m*/
	//loadModel("models\\duck.obj_m", &objModel);
	loadModel("models\\dada.obj_m", &dada1);
	loadModel("models\\papa.obj_m", &papa);


	glActiveTexture(GL_TEXTURE0);
	loadModel("models\\duck.obj_m", &duck);
	duckTex.loadTextureFromFile("textures//fff.bmp");
	duckTex.bindTexture();


	tick_n = GetTickCount();
	tick_o = tick_n;

	rec.setSize(300, 100);
	rec.setPosition(10, ogl->getHeight() - 100 - 10);
	rec.setText("T - ���/���� �������\nL - ���/���� ���������\nF - ���� �� ������\nG - ������� ���� �� �����������\nG+��� ������� ���� �� ���������", 0, 0, 0);


}
void Mountain1()
{
	glPushMatrix();
	glColor4f(0, 1, 0, 1.0f);
	glTranslatef(-3, -1, 1.5);
		GLfloat points[4][4][3] = 
		{
		{{ -1.5, -1.5, -1.5 }, { -0.5, -1.5, -1.5 }, { 0.5, -1.5, -1.5 }, { 1.5, -1.5, -1.5 }},
		{{ -1.5, -0.5, -1.5 }, { -0.5, -0.5, 1.0 }, { 0.5, -0.5, 1.0 }, { 1.5, -0.5, -1.5 }},
		{{ -1.5, 0.5, -1.5 }, { -0.5, 0.5, 1.0 }, { 0.5, 0.5, 1.0 }, { 1.5, 0.5, -1.5 }},
		{{ -1.5, 1.5, -1.5 }, { -0.5, 1.5, -1.5 }, { 0.5, 1.5, -1.5 }, { 1.5, 1.5, -1.5 }}
		};
	
			glMap2f(GL_MAP2_VERTEX_3, 0, 1, 3, 4, 0, 1, 12, 4, &points[0][0][0]);
			glEnable(GL_MAP2_VERTEX_3);
			glMapGrid2f(10, 0, 1, 10, 0, 1);
			glEvalMesh2(GL_FILL, 0, 10, 0, 10);

			glPopMatrix();
}

float roll = 0;
float pitch = 0;
float yaw = 0;


float angle = 0.0f;
const float CIRCLE_RADIUS = 15.0f;
const float ANGULAR_SPEED = 0.01f;

void drawCircle(float radius) 
{
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360; i++) 
	{
		float angle = i * 3.14159f / 180.0f;
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		glVertex3f(x, y, 0.0f);
	}
	glEnd();
}

void drawLake(float radius, int segments) 
{
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0f, 0.0f);  // ����� �����

	for (int i = 0; i <= segments; i++) 
	{
		float angle = 2.0f * 3.14159f * float(i) / float(segments);
		float x = radius * cos(angle);
		float y = radius * sin(angle);
		glVertex2f(x, y);
	}

	glEnd();
}

void DrawDucks(float x, float y, float z,  float angle, float yaw, float pitch)
{
		glTranslatef(x, y, z);
		glRotatef(angle, 0, 0, 1);
		glRotatef(yaw, 0, 0, 1);


		//duck1
		s[1].UseShader();
		int l = glGetUniformLocationARB(s[1].program, "tex");
		glUniform1iARB(l, 0);     //��� ��� ����� �� ��������� �������� ������� �� GL_TEXTURE0
		glPushMatrix();
		glScaled(0.5, 0.5, 0.5);
		glRotated(90, 0, 0, 1);
		duckTex.bindTexture();
		duck.DrawObj();
		glPopMatrix();

}

void Moving(float radius, float angularSpeed)
{
	drawCircle(radius);

	float DuckX = radius * cos(angle);
	float DuckY = radius * sin(angle);
	float DuckAngle = atan2(DuckY, DuckX) * 180 / 3.14159f;
	float yaw = atan2(360.0f, DuckX) * 180 / 3.14159f;

	DrawDucks(DuckX, DuckY, 0.0f, DuckAngle, yaw, pitch);
	if (IsDuck) {
		angle += ANGULAR_SPEED;
	}
	/*angle += ANGULAR_SPEED;*/

	if (angle > 360.0f) 
	{
		angle -= 360.0f;
	}
}

void Render(OpenGL* ogl)
{

	tick_o = tick_n;
	tick_n = GetTickCount();
	Time += (tick_n - tick_o) / 1000.0;

	/*
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	*/

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//��������������
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//��������� ���������
	//GLfloat amb[] = { 0.2, 0.2, 0.1, 1. };
	//GLfloat dif[] = { 0.4, 0.65, 0.5, 1. };
	//GLfloat spec[] = { 0.9, 0.8, 0.3, 1. };
	//GLfloat sh = 0.1f * 256;

	////�������
	//glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	////��������
	//glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	////����������
	//glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
	////������ �����
	//glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//===================================
	//������� ���  

	//lake
	glPushMatrix();
	glTranslated(0, 0, 0.011);
	glColor4f(0.0, 1.0, 1.0, 0.7);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	drawLake(20.0f, 100); 
	glPopMatrix();

	// 
	glPushMatrix();
	Moving(CIRCLE_RADIUS, ANGULAR_SPEED);
	glPopMatrix();
	

	s[0].UseShader();

	//�������� ���������� � ������.  ��� ���� - ���� ����� uniform ���������� �� �� �����. 
	int location = glGetUniformLocationARB(s[0].program, "light_pos");
	//��� 2 - �������� �� ��������
	glUniform3fARB(location, light.pos.X(), light.pos.Y(), light.pos.Z());

	location = glGetUniformLocationARB(s[0].program, "Ia");
	glUniform3fARB(location, 0.2, 0.2, 0.2);

	location = glGetUniformLocationARB(s[0].program, "Id");
	glUniform3fARB(location, 1.0, 1.0, 1.0);

	location = glGetUniformLocationARB(s[0].program, "Is");
	glUniform3fARB(location, .7, .7, .7);


	location = glGetUniformLocationARB(s[0].program, "ma");
	glUniform3fARB(location, 0.2, 0.2, 0.1);

	location = glGetUniformLocationARB(s[0].program, "md");
	glUniform3fARB(location, 0.4, 0.65, 0.5);

	location = glGetUniformLocationARB(s[0].program, "ms");
	glUniform4fARB(location, 0.9, 0.8, 0.3, 25.6);

	location = glGetUniformLocationARB(s[0].program, "camera");
	glUniform3fARB(location, camera.pos.X(), camera.pos.Y(), camera.pos.Z());


	//duck2
	s[1].UseShader();
	int ll = glGetUniformLocationARB(s[1].program, "tex");
	glUniform1iARB(ll, 0);     //��� ��� ����� �� ��������� �������� ������� �� GL_TEXTURE0
	glPushMatrix();
	glScaled(0.5, 0.5, 0.5);
	glRotated(90, 0, 0, 1);
	glTranslated(2, 2, 0);
	duckTex.bindTexture();
	duck.DrawObj();
	glPopMatrix();



	Shader::DontUseShaders();

	glPushMatrix();
	glBegin(GL_QUADS); // �������� �������� ���������������
	glColor4f(0, 1, 0, 0.9f);
	glVertex3f(-100, -100, 0); // ����� ������ �������
	glColor4f(0, 0.5, 0, 0.9f);
	glVertex3f(100, -100, 0); // ������ ������ �������
	glColor4f(0, 1, 0, 0.6f);
	glVertex3f(100, 100, 0); // ������ ������� �������
	glColor4f(0, 0.5, 0, 0.6f);
	glVertex3f(-100, 100, 0); // ����� ������� �������
	glEnd();
	glPopMatrix();


	
	//dada1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.8f);
	glTranslated(-18.5, 0, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 0.5, 0, 0.9f);
	glTranslated(-20, 6, 0);
	glScaled(0.005f, 0.005f, 0.005f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 0.5, 0, 0.8f);
	glTranslated(-18, 9, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix(); 

	//dada1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.9f);
	glTranslated(-15, 15, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 0.5, 0, 0.7f);
	glTranslated(0, 18, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.8f);
	glTranslated(-5, 18, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 0.5, 0, 0.9f);
	glTranslated(-10, 16, 0);
	glScaled(0.001f, 0.001f, 0.001f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 1, 0.5, 0.9f);
	glTranslated(-20, 18, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();

	//dada1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.8f);
	glTranslated(-22, 18, 0);
	glScaled(0.003f, 0.003f, 0.003f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix(); 

	//dada1
	glPushMatrix();
	glColor4f(0, 0.5, 0, 0.7f);
	glTranslated(-23, 18, 0);
	glScaled(0.003f, 0.003f, 0.005f);
	glRotated(90, 90, 0, 1);
	dada1.DrawObj();
	glPopMatrix();


	//papa1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.5f);
	glTranslated(5, 7, 0);
	glScaled(0.07, 0.07, 0.07);
	glRotated(90, 0, 0, 1);
	papa.DrawObj();
	glPopMatrix();

	//papa1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.5f);
	glTranslated(-12, 7, 0);
	glScaled(0.07, 0.07, 0.07);
	glRotated(90, 0, 0, 1);
	papa.DrawObj();
	glPopMatrix();

	//papa1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.5f);
	glTranslated(-9, -9, 0);
	glScaled(0.07, 0.07, 0.07);
	glRotated(90, 0, 0, 1);
	papa.DrawObj();
	glPopMatrix();

	//papa1
	glPushMatrix();
	glColor4f(0, 1, 0, 0.5f);
	glTranslated(15, -9, 0);
	glScaled(0.07, 0.07, 0.07);
	glRotated(90, 0, 0, 1);
	papa.DrawObj();
	glPopMatrix();

	glPushMatrix();
	glScaled(10, 13, 20);
	glRotated(0, 180, 0,0);
	Mountain1();
	glPopMatrix();



	//////��������� ��������


	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		frac.UseShader();

		int location = glGetUniformLocationARB(frac.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());

		location = glGetUniformLocationARB(frac.program, "uOffset");
		glUniform2fARB(location, offsetX, offsetY);

		location = glGetUniformLocationARB(frac.program, "uZoom");
		glUniform1fARB(location, zoom);

		location = glGetUniformLocationARB(frac.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();

	}
	*/


	//////���� �������

	/*
	{

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0,1,0,1,-1,1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		cassini.UseShader();

		int location = glGetUniformLocationARB(cassini.program, "size");
		glUniform2fARB(location, (GLfloat)ogl->getWidth(), (GLfloat)ogl->getHeight());


		location = glGetUniformLocationARB(cassini.program, "Time");
		glUniform1fARB(location, Time);

		DrawQuad();
	}

	*/






	Shader::DontUseShaders();



}   //����� ���� �������


bool gui_init = false;

//������ ���������, ��������� ����� �������� �������
void RenderGUI(OpenGL* ogl)
{

	Shader::DontUseShaders();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);


	glActiveTexture(GL_TEXTURE0);
	rec.Draw();



	Shader::DontUseShaders();




}

void resizeEvent(OpenGL* ogl, int newW, int newH)
{
	rec.setPosition(10, newH - 100 - 10);
}

