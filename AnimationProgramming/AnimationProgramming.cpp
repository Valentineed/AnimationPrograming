// AnimationProgramming.cpp : Defines the entry point for the console application.
//

#include <string>
#include <vector>

#include "Animator.h"
#include "stdafx.h"

#include "Engine.h"
#include "Simulation.h"
#include "Quaternion/Quaternion.h"
#include "Matrix/Matrix4.h"
#include "Skeleton.h"

using namespace LibMath;



class CSimulation : public ISimulation
{
	Skeleton m_skeleton;
	Animator m_animator;
	float m_currentTime;
	float m_timing = 3;
	
	virtual void Init() override
	{
		int spine01 =	GetSkeletonBoneIndex("spine_01");
		int spineParent = GetSkeletonBoneParentIndex(spine01);
		const char* spineParentName = GetSkeletonBoneName(spineParent);

		float posX, posY, posZ, quatW, quatX, quatY, quatZ;
		size_t keyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		GetAnimLocalBoneTransform("ThirdPersonWalk.anim", spineParent, keyCount / 2, posX, posY, posZ, quatW, quatX, quatY, quatZ);

		printf("Spine parent bone : %s\n", spineParentName);
		printf("Anim key count : %ld\n", keyCount);
		printf("Anim key : pos(%.2f,%.2f,%.2f) rotation quat(%.10f,%.10f,%.10f,%.10f)\n", posX, posY, posZ, quatW, quatX, quatY, quatZ);
		InitSkeleton();

		size_t animKeyCount = GetAnimKeyCount("ThirdPersonRun.anim");
		m_animator.AddNewAnimation("ThirdPersonRun.anim", animKeyCount, &m_skeleton);
		 animKeyCount = GetAnimKeyCount("ThirdPersonWalk.anim");
		m_animator.AddNewAnimation("ThirdPersonWalk.anim", animKeyCount, &m_skeleton);
	}
	
	void InitSkeleton()
	{
		for (int BoneIndex = 0; BoneIndex < (int)GetSkeletonBoneCount(); BoneIndex++)
		{
			Vector3 pos;
			Quaternion rot;

			GetSkeletonBoneLocalBindTransform(BoneIndex, pos.x, pos.y, pos.z, rot.W, rot.X, rot.Y, rot.Z);
			Matrix4 translateMatrix = Matrix4::Identity();
			translateMatrix.Translate(pos);

			Matrix4 rotationMatrix = Matrix4::Identity();
			rotationMatrix.Rotate(rot);

			Matrix4 localMatrix = translateMatrix * rotationMatrix;
			int parentBoneIndex = GetSkeletonBoneParentIndex(BoneIndex);

			Matrix4 globalMatrix = localMatrix;
			if (BoneIndex > 0)
			{
				globalMatrix = m_skeleton.GetBone(parentBoneIndex)->m_globalTransform * localMatrix;
			}
			m_skeleton.AddBone(localMatrix, globalMatrix, parentBoneIndex, GetSkeletonBoneName(BoneIndex));
		}
	}
	
	void DrawAnimation(float frameTime)
	{
		ChangeAnimation(frameTime);
		int keyFrameindex = m_animator.UpdateAnimator(frameTime);
		//DebugDrawAnimation(keyFrameindex);
	}
	
	void DebugDrawAnimation(float keyFrameindex)
	{
		for (int boneIndex = 1; boneIndex < (int)GetSkeletonBoneCount(); boneIndex++)
		{
			const Bone* bone = m_skeleton.GetBone(boneIndex);
			if (bone == nullptr || bone->IsIK())
			{
				return;
			}

			SQT* sqt = m_animator.GetAnimationClip().GetTransforms(keyFrameindex, boneIndex);
			SQT* sqtParent = m_animator.GetAnimationClip().GetTransforms(keyFrameindex,bone->m_parentIndex);
			if(sqt == nullptr || sqtParent == nullptr)
			{
				return;
			}

			DrawLine(sqt->translation.x, sqt->translation.y - 100, sqt->translation.z,
				sqtParent->translation.x, sqtParent->translation.y - 100, sqtParent->translation.z, 0.5, 0.5, 0.5);
		}
	}

	int currentAnimation = 1;
	void ChangeAnimation(float frameTime)
	{
		m_currentTime += frameTime;
		if (m_timing <= m_currentTime)
		{
			if(currentAnimation > 1)
			{
				currentAnimation = 0;
			}
			m_animator.ChangeAnimation(currentAnimation);
			currentAnimation++;
			m_timing = 20.f;
			m_currentTime = 0.0f;
		}
	}
	void DrawSkeleton()
	{
		for (int index = 1; index < m_skeleton.GetBoneCount(); index++)
		{
			const Bone* bone = m_skeleton.GetBone(index);
			if(bone == nullptr || bone->IsIK())
			{
				return;
			}

			const_row startPoint = bone->m_globalTransform[3];
			const_row endPoint = m_skeleton.GetBone(bone->m_parentIndex)->m_globalTransform[3];

			DrawLine(startPoint[0], startPoint[1] - 100, startPoint[2], endPoint[0], endPoint[1] - 100, endPoint[2], 0.5, 0.5, 0.5);
		}
	}
	
	virtual void Update(float frameTime) override
	{
		// X axis
		DrawLine(0, 0, 0, 100, 0, 0, 1, 0, 0);

		// Y axis
		DrawLine(0, 0, 0, 0, 100, 0, 0, 1, 0);

		// Z axis
		DrawLine(0, 0, 0, 0, 0, 100, 0, 0, 1);

		//DrawSkeleton();
		DrawAnimation(frameTime);
	}
};

int main()
{
	CSimulation simulation;
	Run(&simulation, 1400, 800);

	return 0;
}

