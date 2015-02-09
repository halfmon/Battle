#pragma once

#include <vector>

#include "Defines.h"
#include "Entity.h"

namespace gen
{

enum EItemEffect
{
	restoreHealth,
	restoreMana,
	poison,
	revive
};

struct SItem
{
	std::string name;
	EItemEffect effect;
	TInt32 value;
};

const SItem POTION = { "Potion", restoreHealth, 30 };
const SItem SUPER_POTION = { "Super Potion", restoreHealth, 70 };

const SItem MAGIC_POTION = { "Magic Potion", restoreMana, 40 };
const SItem SUPER_MAGIC_POTION = { "Super Magic Potion", restoreMana, 90 };

const SItem VENOM = { "Venom", poison, 10 };

const SItem REVIVE = { "Revive", revive, 50 };

class CItem
{
private:
	std::string m_Name;
	EItemEffect m_Effect;
	TInt32 m_Value;

public:
	CItem(std::string name,EItemEffect effect,TInt32 value);

	SItem Use(TEntityUID user);

	EItemEffect GetEffect()
	{
		return m_Effect;
	}

	std::string GetName()
	{
		return m_Name;
	}
};
}