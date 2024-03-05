#pragma once
#include <vector>

using namespace LibMath;
struct Bone
{
public:
	Bone(Matrix4 local, Matrix4 global, unsigned int indexParent, const char* name) :
		m_localTransform(local), m_globalTransform(global), m_parentIndex(indexParent), m_name(name)
	{
		m_inverseGlobal = m_globalTransform.GetInverse();
	}
	bool IsIK() const
	{
		return m_name.find("ik_") == 0;
	}
	std::string m_name;
	unsigned int m_parentIndex = 0;
	Matrix4 m_localTransform;
	Matrix4 m_globalTransform;
	Matrix4 m_inverseGlobal;
};

class Skeleton
{
public:
	
	const Bone* GetBone(unsigned int index) const
	{
		if (index < m_bones.size())
		{
			return &m_bones[index];
		}
		return nullptr;
	}

	void AddBone(LibMath::Matrix4 local, LibMath::Matrix4 global, unsigned int indexParent, const char* name)
	{
		m_bones.emplace_back(local, global, indexParent, name);
	}

	[[nodiscard]] int GetBoneCount()const 
	{
		return (int)m_bones.size();
	}

	
	std::vector<Bone> m_bones;
};
