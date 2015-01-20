#pragma once

#include "Defines.h"
#include "Attacks.h"

namespace gen
{

enum EDefenceType
{
	Regular,
	Reflect,
	PainSplit
};

struct SDefence
{
	EDefenceType type;
	EAttackType  attackRecivedType;
	EElement     element;
	float        modifier;
	int          cost;
};

const SDefence BASIC_PHYSICAL = { Regular, Physical, None, 0.75f, 0 };
const SDefence BASIC_MAGICAL  = { Regular, Magical, None, 0.75f, 0 };

const SDefence ADVANCED_PHYSICAL = { Regular, Physical, None, 0.5f, 20 };
const SDefence ADVANCED_MAGICAL =  { Regular, Magical, None, 0.5f, 20 };

const SDefence LIGHTNING_DEFENCE = { Regular, Magical, Lightning, 0.25f, 30 };
const SDefence FIRE_DEFENCE =      { Regular, Magical, Fire, 0.25f, 30 };
const SDefence ICE_DEFENCE =       { Regular, Magical, Ice, 0.25f, 30 };
const SDefence ARCANE_DEFENCE =    { Regular, Magical, Arcane, 0.25f, 30 };

const SDefence REFLECT = { Reflect, Both, None, 0.5f, 70 };
const SDefence PAIN_SPLIT = { PainSplit, Both, None, 0.5f, 35 };

}// namespace gen