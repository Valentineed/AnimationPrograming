#pragma once
#include <vector>
#include "AnimationClip.h"
#include "iostream"

class Animator
{
private:
public:

	void ChangeAnimation(int animationKey);

	int UpdateAnimator(float frameTime);

	void Interpolation(float frameTime);

	SQT InterpolateKeyFrames(int animationIndex, int keyFrameIndex, int maxKeyframes, int boneIndex, float alpha);

	void AddNewAnimation(const char* name, int animKeyCount, Skeleton* skeleton);

	AnimationClip& GetAnimationClip();

	bool AnimationIsChange();
private:
	int m_oldAnimation = -1.f;
	bool m_animationChange = false;
	int m_currentKeyAnimation = 0;
	std::vector<AnimationClip> m_animationClips;
	Skeleton* m_skeleton;
	int m_currentKey;

	float m_blendingRatio = 0.0f;
	const float m_timeToBlendReset = 1.f;
	float m_timeToBlend = m_timeToBlendReset;

	AnimationClip* m_oldAnimationClip;
	AnimationClip* m_nextAnimationClip;
};

