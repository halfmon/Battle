#pragma once

#include <vector>

#include "Defines.h"
#include "Entity.h"

namespace gen
{

enum EAttackType
{
	Physical,
	Magical,
	Both
};
enum EAttackElement
{
	Cut,
	Crush,
	Stab,
	Lightning,
	Fire,
	Ice,
	Arcane,
	None
};

struct SWeakness
{
	EAttackElement element;
	TFloat32       modifier;
	TInt32         turns;
};

struct SAttack
{
	EAttackType       type;
	EAttackElement    element;
	TInt32            cost;
	TFloat32          damage;
};


const SAttack CUT =       { Physical, Cut, 0, 12 };
const SAttack CRUSH =     { Physical, Crush, 0, 8 };
const SAttack STAB =      { Physical, Stab, 0, 10 };
const SAttack LIGHTNING = { Magical, Lightning, 12, 16 };
const SAttack FIRE =      { Magical, Fire, 5, 7 };
const SAttack ICE =       { Magical, Ice, 9, 10 };
const SAttack ARCANE =    { Magical, Arcane, 10, 12 };

class CAttack
{
private:
	std::string       m_Name;
	EAttackType       m_Type;
	EAttackElement    m_Element;
	TInt32            m_Cost;
	TFloat32          m_Damage;
	vector<SWeakness> m_AddWeakness;
	TFloat32          m_Recoil;

public:
	CAttack(std::string name,EAttackType type, EAttackElement element, TInt32 cost, TFloat32 damage, vector<SWeakness> addWeakness, TFloat32 recoil);

	SAttack Attack(TEntityUID attacker, TEntityUID target);

	std::string GetName()
	{
		return m_Name;
	}
	TFloat32 GetDamage()
	{
		return m_Damage;
	}
};

}// namespace gen