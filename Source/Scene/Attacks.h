#pragma once

#include "Defines.h"

namespace gen
{

enum EAttackType
{
	Physical,
	Magical,
	Both
};
enum EElement
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
;
struct SAttack
{
	EAttackType type;
	EElement    element;
	TInt32      cost;
	TFloat32    damage;
};


const SAttack CUT =       { Physical, Cut, 0, 12 };
const SAttack CRUSH =     { Physical, Crush, 0, 8 };
const SAttack STAB =      { Physical, Stab, 0, 10 };
const SAttack LIGHTNING = { Magical, Lightning, 12, 16 };
const SAttack FIRE =      { Magical, Fire, 5, 7 };
const SAttack ICE =       { Magical, Ice, 9, 10 };
const SAttack ARCANE =    { Magical, Arcane, 10, 12 };

}// namespace gen