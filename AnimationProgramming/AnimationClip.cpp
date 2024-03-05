#include "AnimationClip.h"
#include "Engine.h"
#include <Interpolation.h>

AnimationClip::AnimationClip(const char* name, size_t animKeyCount, Skeleton* skeleton) :
	m_name(name), m_keyCount(animKeyCount)
{
	m_duration = animKeyCount / 30.f;
	m_keyTimeFrame = m_duration / (float)animKeyCount;
	m_skeleton = skeleton;
	m_keyframes.reserve(animKeyCount);
	if (m_skeleton == nullptr)
	{
		std::cerr << "Skeleton is nullptr" << std::endl;
	}
	InitKeyFrames();
}

int AnimationClip::UpdateTimingAndAlpha(float frameTime)
{
	m_currentTime += frameTime;
	m_interpolate = m_currentTime / m_keyTimeFrame;
	if (m_currentTime >= m_keyTimeFrame)
	{
		m_currentTime =  m_currentTime - m_keyTimeFrame;
		m_currentKey = (m_currentKey + 1) % (int)m_keyCount;
		m_interpolate = 0.0f;
	}
	return  m_currentKey;
}

void AnimationClip::CreateMatricesToGPU()
{
	for (int boneIndex = 0; boneIndex < (int)m_skeleton->m_bones.size(); boneIndex++)
	{
		const Bone* bone = m_skeleton->GetBone(boneIndex);
		if (bone == nullptr || bone->IsIK())
		{
			break;
		}
		const SQT& sqt = m_keyframes[m_currentKey].m_sqt[boneIndex];
		int nextSqtIndex = 0;
		if (m_currentKey == (int)(m_keyframes.size() - 1))
		{
			nextSqtIndex = 0;
		}
		else
		{
			nextSqtIndex = m_currentKey + 1;
		}
		const SQT& sqtNext = m_keyframes[nextSqtIndex].m_sqt[boneIndex];

		SQT newSQT = Interpole(sqt, sqtNext, m_interpolate);

		Matrix4 localMatrix = Matrix4::Identity();
		localMatrix.Rotate(newSQT.rotation);
		localMatrix.Translate(newSQT.translation);

		localMatrix *= bone->m_inverseGlobal;
		m_matricesToGPU.push_back(localMatrix);
	}
}

SQT AnimationClip::Interpole(SQT const& start, SQT const& end, float alpha)
{
	Vector3 translation = Interpolation::Lerp(start.translation, end.translation, alpha);
	Quaternion rotation = Interpolation::Slerp(start.rotation, end.rotation, alpha);

	return SQT(translation, rotation);
}

void AnimationClip::SendMatricesToGPU()
{
	SetSkinningPose((float*)m_matricesToGPU.data(), m_matricesToGPU.size());
	m_matricesToGPU.clear();
}

void AnimationClip::ChangeTimeLine(float multiplyTimeLine)
{
	if (multiplyTimeLine < 0)
	{
		multiplyTimeLine = 1.f;
	}
	m_multiplyTime = multiplyTimeLine;
	m_keyTimeFrame *= m_multiplyTime;
}

SQT* AnimationClip::GetTransforms(int KeyFrame, int boneIndex) 
{
	if(KeyFrame < 0 || boneIndex < 0 || KeyFrame > (int)m_keyframes.size() -1)
	{
		std::cerr << "invalid index" << std::endl;
		return nullptr;
	}
	else if(boneIndex > (int)m_keyframes[KeyFrame].m_sqt.size() - 1)
	{
		return nullptr;
	}
	return &m_keyframes[KeyFrame].m_sqt[boneIndex];
}

void AnimationClip::InitKeyFrames()
{
	for (size_t keyFrameIndex = 0; keyFrameIndex < m_keyCount; keyFrameIndex++)
	{
		KeyFrame& keyframe = m_keyframes.emplace_back(keyFrameIndex);
		InitKeyFrame(keyframe);
	}
}

void AnimationClip::InitKeyFrame(KeyFrame& keyframe)
{
	int boneCount = GetSkeletonBoneCount();
	keyframe.m_sqt.reserve(boneCount);
	for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
	{
		const Bone* bone = m_skeleton->GetBone(boneIndex);
		if (bone == nullptr || bone->IsIK())
		{
			return;
		}
		SQT bindPoseSQT;
		GetSkeletonBoneLocalBindTransform(boneIndex, bindPoseSQT.translation.x, bindPoseSQT.translation.y, bindPoseSQT.translation.z,
			bindPoseSQT.rotation.W, bindPoseSQT.rotation.X, bindPoseSQT.rotation.Y, bindPoseSQT.rotation.Z);

		SQT animSQT;
		GetAnimLocalBoneTransform(m_name.c_str(), boneIndex, keyframe.index,
			animSQT.translation.x, animSQT.translation.y, animSQT.translation.z,
			animSQT.rotation.W, animSQT.rotation.X, animSQT.rotation.Y, animSQT.rotation.Z);

		animSQT = animSQT * bindPoseSQT;
		if (boneIndex > 0)
		{
			const SQT& parentSQT = keyframe.m_sqt[bone->m_parentIndex];
			animSQT = animSQT * parentSQT;
		}
		keyframe.m_sqt.emplace_back(animSQT);
	}
}
