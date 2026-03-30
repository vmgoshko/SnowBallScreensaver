#include <vector>
#include <set>
#include <cmath>
#include <windows.h>
using std::vector;

struct stPoint
{
	float mX;
	float mY;
	float mZ;

	stPoint(float x = 0, float y = 0, float z = 0) : 
	mX(x),mY(y),mZ(z)
	{

	}

	stPoint operator *(stPoint& r){
		return stPoint(this->mX *r.mX,
			this->mY *r.mY,
			this->mZ *r.mZ);
	}

	stPoint operator /(float alpha){
		return stPoint(this->mX / alpha,
			this->mY / alpha,
			this->mZ / alpha);
	}

	stPoint operator *(float alpha){
		return stPoint(this->mX * alpha,
			this->mY * alpha,
			this->mZ * alpha);
	}

	stPoint operator +=(stPoint& r){
		this->mX += r.mX;
		this->mY += r.mY;
		this->mZ += r.mZ;
		return *this;
	}

	float norma()
	{
		return sqrt(mX*mX + mY*mY+ mZ*mZ);
	}

};

struct stSnowFlake {
	stPoint mPos;
	stPoint mDir;
};

struct stProperties{
	int mFlakesCount;
	float mFlakesVelocity;
	float mRotateSpeed;
};

class CScene
{
private:
	const float PI;
	vector<unsigned int> mTextures;
	vector<stSnowFlake> mFlakes;
	int mFlakesCount;

	int mXSize;
	int mYSize;
	float mSphereTranslateZ;

	float mZMax;
	float mSphereRadius;
	float mFlakesVelocity;
	float mRotateSpeed;
public: 
	CScene(stProperties& props);
	void Render(HDC hDC);
private:
	void TranslateFlakes();
	void CreateFlake();
	void InitFlakes();
	void LoadTexture(const char* name);

	void RenderFlake(stSnowFlake& flake);
	void RenderFlakes();
	void RenderCircle(float radius);
	void RenderGlassSphere();
	void RenderTree();
	void RenderStand();
};