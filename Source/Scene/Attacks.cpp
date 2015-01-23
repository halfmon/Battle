#include "Attacks.h"

namespace gen
{

CAttack::CAttack(std::string name,EAttackType type,EAttackElement element,TInt32 cost,TFloat32 damage,vector<SWeakness> addWeakness, TFloat32 recoil)
{
	m_Name = name;
	m_Type = type;
	m_Element = element;
	m_Cost = cost;
	m_Damage = damage;
	m_Recoil = recoil;

	for(auto it = addWeakness.begin(); it != addWeakness.end(); it++)
	{
		m_AddWeakness.push_back(*it);
	}
}

SAttack CAttack::Attack(TEntityUID attacker,TEntityUID target)
{
	return LIGHTNING;
}

}