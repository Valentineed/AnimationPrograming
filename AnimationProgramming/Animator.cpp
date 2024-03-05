
#include "Animator.h"

void Animator::ChangeAnimation(int animationKey)
{
	if (animationKey > m_animationClips.size() - 1 || animationKey < 0)
	{
		std::cerr << "Key no Valid in Animation (Change Animation)" << std::endl;
		return;
	}

	m_oldAnimation = m_currentKeyAnimation;
	m_currentKeyAnimation = animationKey;
	if (m_oldAnimation != -1)
	{
		m_animationChange = true;
		m_oldAnimationClip = &m_animationClips[m_oldAnimation];
		m_nextAnimationClip = &m_animationClips[m_currentKeyAnimation];
	}
}

int Animator::UpdateAnimator(float frameTime)
{
	if (m_animationChange)
	{
		m_oldAnimationClip->UpdateTimingAndAlpha(frameTime); //Current update Values
		Interpolation(frameTime);
	}
	else
	{
		m_currentKey = m_animationClips[m_currentKeyAnimation].UpdateTimingAndAlpha(frameTime);
		m_animationClips[m_currentKeyAnimation].CreateMatricesToGPU();
		m_animationClips[m_currentKeyAnimation].SendMatricesToGPU();
	}
	return m_currentKey;
}

void Animator::Interpolation(float frameTime)
{
	m_timeToBlend -= frameTime;
	if (m_timeToBlend <= 0.0f)
	{
		m_blendingRatio += 0.1f;
		m_timeToBlend = m_timeToBlendReset;
	}
	if (m_blendingRatio >= 1.0f)
	{
		m_animationChange = false;
		m_blendingRatio = 0.0f;
	}

	const int KeyCountSource = m_oldAnimationClip->m_keyCount;
	const int keyCountDest = m_nextAnimationClip->m_keyCount;
	const float timeScale = (float)keyCountDest / (float)KeyCountSource;
	const int keyPlayedFirstAnim = m_oldAnimationClip->m_currentKey;
	const float InterpolateValueFirstAnim = m_oldAnimationClip->m_interpolate;
	const int keyNext = ((float)keyPlayedFirstAnim + InterpolateValueFirstAnim) * timeScale;
	float interpolateNext = (((float)keyPlayedFirstAnim + InterpolateValueFirstAnim) * timeScale) - keyNext;

	for (int boneIndex = 0; boneIndex < (int)m_skeleton->m_bones.size(); boneIndex++)
	{
		const Bone* bone = m_skeleton->GetBone(boneIndex);
		if (bone == nullptr || bone->IsIK())
		{
			break;
		}

		//Interpolate In CurrentAnim
		int indexCount = (m_oldAnimationClip->m_keyframes.size() - 1);
		SQT currentAnim = InterpolateKeyFrames(m_oldAnimation, m_oldAnimationClip->m_currentKey, indexCount, boneIndex,
		                                     m_oldAnimationClip->m_interpolate);

		//Interpole NextAnim
		indexCount = (m_nextAnimationClip->m_keyframes.size() - 1);
		SQT nextAnim = InterpolateKeyFrames(m_currentKeyAnimation, keyNext, indexCount, boneIndex, interpolateNext);

		SQT result = m_oldAnimationClip->Interpole(currentAnim, nextAnim, m_blendingRatio);


		Matrix4 localMatrix = Matrix4::Identity();
		localMatrix.Rotate(result.rotation);
		localMatrix.Translate(result.translation);

		localMatrix *= bone->m_inverseGlobal;
		m_oldAnimationClip->m_matricesToGPU.push_back(localMatrix);
	}
	m_oldAnimationClip->SendMatricesToGPU();
}

SQT Animator::InterpolateKeyFrames(int animationIndex, int keyFrameIndex, int maxKeyframes, int boneIndex, float alpha)
{
	const SQT& sqt = m_animationClips[animationIndex].m_keyframes[keyFrameIndex].m_sqt[boneIndex];
	int nextKeyIndex = 0;
	if (keyFrameIndex == maxKeyframes)
	{
		nextKeyIndex = 0;
	}
	else
	{
		nextKeyIndex = keyFrameIndex + 1;
	}
	const SQT& sqtNext = m_animationClips[animationIndex].m_keyframes[nextKeyIndex].m_sqt[boneIndex];
	return m_animationClips[animationIndex].Interpole(sqt, sqtNext, alpha);
}

void Animator::AddNewAnimation(const char* name, int animKeyCount, Skeleton* skeleton)
{
	m_skeleton = skeleton;
	m_animationClips.emplace_back(name, animKeyCount, skeleton);
}

AnimationClip& Animator::GetAnimationClip()
{
	return m_animationClips[m_currentKeyAnimation];
}

bool Animator::AnimationIsChange()
{
	return m_animationChange;
}
