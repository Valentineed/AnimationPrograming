#pragma once
#include <string>
#include <vector>
#include <Matrix/Matrix4.h>
#include <Quaternion/Quaternion.h>
#include "Skeleton.h"
#include <iostream>
using namespace LibMath;
struct SQT
{
	SQT() = default;

	SQT(const SQT& other) : translation(other.translation), rotation(other.rotation) {}

	SQT(Vector3 translation, Quaternion rotation) : translation(translation), rotation(rotation)
	{	}

	SQT operator*(const SQT& other)const
	{
		SQT newsqt;
		newsqt.translation = (other.rotation * this->translation) + other.translation;
		newsqt.rotation = other.rotation * this->rotation;
		newsqt.rotation.Normalize();
		return newsqt;
	}
	Vector3 translation;
	Quaternion rotation;
};
class KeyFrame
{
public:
	KeyFrame(const int keyindex) :index(keyindex) {}
	int index;
	std::vector<SQT> m_sqt;
};

class AnimationClip
{
public:
	AnimationClip() = default;
	
	AnimationClip(const char* name, size_t animKeyCount, Skeleton* skeleton);

	int UpdateTimingAndAlpha(float frameTime);
	
	void CreateMatricesToGPU();

	SQT Interpole(SQT const& start, SQT const& end, float alpha);

	void SendMatricesToGPU();

	void ChangeTimeLine(float multiplyTimeLine);

	SQT* GetTransforms(int KeyFrame, int boneIndex);
	
	std::string m_name;
	size_t m_keyCount;
	float m_keyTimeFrame;
	int m_currentKey = 0;
	float m_duration = 0.f;
	std::vector <KeyFrame> m_keyframes;
	float m_interpolate = 0.f;
	std::vector<Matrix4> m_matricesToGPU;
private:

	Skeleton* m_skeleton = nullptr;
	float m_multiplyTime = 1.f;
	float m_currentTime = 0.0f;
	float m_lastTime = 0.f;
	float m_deltaTime = 0.f;
	
	void InitKeyFrames();
	void InitKeyFrame(KeyFrame& keyframe);
};