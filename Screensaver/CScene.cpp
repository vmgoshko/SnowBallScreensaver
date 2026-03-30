#include "CScene.h"
#include <windows.h>
#include <time.h>
#include <algorithm>
#include <cmath>
#include <gl\gl.h>
#include <gl\glu.h>			// Header File For The GLu32 Library
#include <iostream>
using std::cout;
using std::endl;

static float l_fPosition[] = { 0.0f, 0.0f, 55.0f, 1.0f };

void EnableFog();
namespace
{
	bool LoadBitmapTextureData(const char* name, std::vector<unsigned char>& pixels, GLsizei& width, GLsizei& height)
	{
		HBITMAP bitmap = static_cast<HBITMAP>(LoadImageA(nullptr, name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION));
		if (bitmap == nullptr)
			return false;

		BITMAP bitmapInfo = {};
		if (GetObject(bitmap, sizeof(bitmapInfo), &bitmapInfo) == 0)
		{
			DeleteObject(bitmap);
			return false;
		}

		BITMAPINFO info = {};
		info.bmiHeader.biSize = sizeof(info.bmiHeader);
		info.bmiHeader.biWidth = bitmapInfo.bmWidth;
		info.bmiHeader.biHeight = -bitmapInfo.bmHeight;
		info.bmiHeader.biPlanes = 1;
		info.bmiHeader.biBitCount = 24;
		info.bmiHeader.biCompression = BI_RGB;

		width = static_cast<GLsizei>(bitmapInfo.bmWidth);
		height = static_cast<GLsizei>(bitmapInfo.bmHeight);
		pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * 3u);

		HDC screenDc = GetDC(nullptr);
		const int scanLines = GetDIBits(screenDc, bitmap, 0, static_cast<UINT>(height), pixels.data(), &info, DIB_RGB_COLORS);
		ReleaseDC(nullptr, screenDc);
		DeleteObject(bitmap);

		if (scanLines == 0)
		{
			pixels.clear();
			return false;
		}

		for (size_t i = 0; i + 2 < pixels.size(); i += 3)
		{
			std::swap(pixels[i], pixels[i + 2]);
		}

		return true;
	}
}

CScene::CScene(stProperties& props) 
	: PI(static_cast<float>(acos(-1.0)))
{
	float scale = 1.5;
	mFlakesCount = props.mFlakesCount;
	mFlakesVelocity = props.mFlakesVelocity;
	mRotateSpeed = props.mRotateSpeed;
	mXSize = 40*2;
	mYSize = 40*2;
	mZMax = 30;
	mSphereRadius = 40;
	mSphereTranslateZ = 15;
	LoadTexture("textures\\tree.bmp");
	LoadTexture("textures\\wood.bmp");
	LoadTexture("textures\\glass.bmp");
	LoadTexture("textures\\stand_wood.bmp");
	LoadTexture("textures\\stand_wood_front.bmp");
	gluLookAt(-100*scale,-30*scale,40*scale,0,0,0,0,0,1);

	InitFlakes();
	TranslateFlakes();
}

void CScene::Render(HDC hDC)
{
	glRotatef(mRotateSpeed,0,0,1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glLightfv( GL_LIGHT0, GL_POSITION, l_fPosition );

	RenderStand();
	RenderTree();
	//RenderPlane();
	RenderFlakes();
	RenderCircle(37);
	RenderGlassSphere();
	TranslateFlakes();
	SwapBuffers(hDC);
}

void CScene::LoadTexture(const char* name)
{
	GLuint tempTexture;
	std::vector<unsigned char> pixels;
	GLsizei width = 0;
	GLsizei height = 0;
	if (LoadBitmapTextureData(name, pixels, width, height))
	{
		glGenTextures( 1, &tempTexture );
		glBindTexture( GL_TEXTURE_2D, tempTexture);

		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR );

		glTexImage2D( GL_TEXTURE_2D, 0, 3, width, height, 0,
			GL_RGB, GL_UNSIGNED_BYTE, pixels.data() );
	}
	else
	{
		tempTexture = 0;
	}

	mTextures.push_back(tempTexture);
}

void CScene::RenderFlake( stSnowFlake& flake )
{
	float flakeSize = 1;
	float x = flake.mPos.mX;
	float y = flake.mPos.mY;
	float z = flake.mPos.mZ;

	if(x*x+y*y+(z-mSphereTranslateZ)*(z-mSphereTranslateZ) > mSphereRadius * mSphereRadius)
		return;

	float color[] = {1,1,1,1};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	glLineWidth(0.03f);
	glBegin(GL_LINES);
	glVertex3f(x - flakeSize/2, y, z);
	glVertex3f(x + flakeSize/2, y, z);
	glVertex3f(x,y - flakeSize/2,z);
	glVertex3f(x,y + flakeSize/2,z);

	glVertex3f(x - flakeSize/3, y - flakeSize/3, z);
	glVertex3f(x + flakeSize/3, y + flakeSize/3, z);
	glVertex3f(x - flakeSize/3, y + flakeSize/3,z);
	glVertex3f(x + flakeSize/3, y - flakeSize/3,z);
	
	glVertex3f(x, y - flakeSize/2, z);
	glVertex3f(x, y + flakeSize/2, z);
	glVertex3f(x, y, z - flakeSize/2);
	glVertex3f(x, y, z + flakeSize/2);

	glVertex3f(x, y - flakeSize/3, z - flakeSize/3);
	glVertex3f(x , y + flakeSize/3, z+ flakeSize/3);
	glVertex3f(x, y + flakeSize/3,z - flakeSize/3);
	glVertex3f(x, y - flakeSize/3,z + flakeSize/3);
	glEnd();
}

void CScene::CreateFlake()
{
	stSnowFlake flake = {stPoint(static_cast<float>(rand()%mXSize - mXSize/2), static_cast<float>(rand()%mYSize - mYSize/2), mZMax + static_cast<float>(rand()%100)),stPoint(0,0,-1)};
	mFlakes.push_back(flake);
}

void CScene::InitFlakes()
{
	for(int i = 0; i < mFlakesCount; i++)
		CreateFlake();
}

void CScene::TranslateFlakes()
{
	for(size_t i = 0; i < mFlakes.size(); ){
		 mFlakes[i].mPos += mFlakes[i].mDir * mFlakesVelocity;
		 
		 mFlakes[i].mPos.mX += static_cast<float>(sin( mFlakes[i].mPos.mZ)/15.0);
		 mFlakes[i].mPos.mY += static_cast<float>(cos( mFlakes[i].mPos.mZ)/15.0);
		
		 if(mFlakes[i].mPos.mZ < 0){
			mFlakes.erase(mFlakes.begin() + static_cast<vector<stSnowFlake>::difference_type>(i));
			CreateFlake();
			continue;
		 }
		 ++i;
	}
}

void CScene::RenderCircle( float radius )
{
	float step = .1f;
	float angle = 0;
	float color[] = {1,1,1,1};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	glBegin(GL_POLYGON);
	while(angle < 2 * PI){
		glNormal3f(0,0,1);
		glVertex3f(radius*cos(angle),radius*sin(angle),0);
		angle += step;
	}
	
	glEnd();
}

void CScene::RenderGlassSphere()
{
	//Drawing glass sphere
	GLdouble equation[4] = {0,0,1,0};

	glEnable(GL_CLIP_PLANE0);
	glClipPlane(GL_CLIP_PLANE0, equation); 

	GLUquadric* quad = gluNewQuadric();

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,mTextures[2]);
	gluQuadricTexture(quad, GLU_TRUE);

	float color_sphere[] = {1,1,1,0.1f};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color_sphere);

	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	glTranslatef(0,0,mSphereTranslateZ);
	gluSphere(quad,mSphereRadius,100,100);
	glPopMatrix();

	glDisable(GL_BLEND); 
	glDisable(GL_TEXTURE_2D);

	gluDeleteQuadric(quad);
	glDisable(GL_CLIP_PLANE0);
}

void CScene::RenderTree()
{

	//Drawing tree
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,mTextures[0]);
	GLUquadric* quad = gluNewQuadric();
	float color[] = {1,1,1,0.5};
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,color);
	glPushMatrix();
	glTranslatef(0,0,22);
	gluQuadricDrawStyle(quad, GLU_FILL);
	gluQuadricTexture(quad, GL_TRUE);
	gluCylinder(quad,8,0,17,50,50);
	glTranslatef(0,0,-9);
	gluCylinder(quad,11,0,20,50,50);
	glTranslatef(0,0,-8);
	gluCylinder(quad,13,0,20,50,50);
	glTranslatef(0,0,-5);
	glBindTexture(GL_TEXTURE_2D,mTextures[1]);
	gluCylinder(quad,3,3,20,50,50);
	glPopMatrix();
	gluDeleteQuadric(quad);

	glDisable(GL_TEXTURE_2D);

}

void CScene::RenderFlakes()
{
	//Drawing snow flakes
	for(size_t i = 0; i < mFlakes.size(); i++){
		RenderFlake(mFlakes[i]);
	}
}

void CScene::RenderStand()
{
	glBindTexture(GL_TEXTURE_2D,mTextures[4]);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(0,0,-1);

	glBegin(GL_QUADS);
	//front
	glTexCoord2f(1,0); glNormal3f(-1,0,0);	glVertex3f(-40,-40,-10);
	glTexCoord2f(0,0); glNormal3f(-1,0,0);	glVertex3f(-40,40,-10);
	glTexCoord2f(0,1); glNormal3f(-1,0,0);	glVertex3f(-40,40,0);
	glTexCoord2f(1,1); glNormal3f(-1,0,0);	glVertex3f(-40,-40,0);


	glBindTexture(GL_TEXTURE_2D, mTextures[3]);
	//up
	glTexCoord2f(0,0); glVertex3f(-40,-40,0);
	glTexCoord2f(1,0); glVertex3f(40,-40,0);
	glTexCoord2f(1,1); glVertex3f(40,40,0);
	glTexCoord2f(0,1); glVertex3f(-40,40,0);
	
	//bottom
	glTexCoord2f(0,0); glNormal3f(0,0,-1);	glVertex3f(-40,-40,-10);
	glTexCoord2f(1,0); glNormal3f(0,0,-1);	glVertex3f(40,-40,-10);
	glTexCoord2f(1,1); glNormal3f(0,0,-1);	glVertex3f(40,40,-10);
	glTexCoord2f(0,1); glNormal3f(0,0,-1);	glVertex3f(-40,40,-10);

	//back
	glTexCoord2f(0,0); glNormal3f(1,0,0);	glVertex3f(40,-40,-10);
	glTexCoord2f(0.25,0); glNormal3f(1,0,0);	glVertex3f(40,40,-10);
	glTexCoord2f(0.25,0.25); glNormal3f(1,0,0);	glVertex3f(40,40,0);
	glTexCoord2f(0,0.25); glNormal3f(1,0,0);	glVertex3f(40,-40,0);

	//left
	glTexCoord2f(0,0);       glNormal3f(0,1,0);	glVertex3f(-40,40,-10);
	glTexCoord2f(0.25,0);    glNormal3f(0,1,0);	glVertex3f(40,40,-10);
	glTexCoord2f(0.25,0.25); glNormal3f(0,1,0);	glVertex3f(40,40,0);
	glTexCoord2f(0,0.25);	 glNormal3f(0,1,0);	glVertex3f(-40,40,0);

	//right
	glTexCoord2f(0,0);		 glNormal3f(0,-1,0);	glVertex3f(-40,-40,-10);
	glTexCoord2f(0.25,0);	 glNormal3f(0,-1,0);	glVertex3f(40,-40,-10);
	glTexCoord2f(0.25,0.25); glNormal3f(0,-1,0);	glVertex3f(40,-40,0);
	glTexCoord2f(0,0.25);	 glNormal3f(0,-1,0);	glVertex3f(-40,-40,0);
	
	glEnd();
	glPopMatrix();
	glDisable(GL_TEXTURE_2D);
}
