#pragma once

#include "Defines.h"

namespace gen
{

enum EEffect
{
	restoreHealth,
	restoreMana,
	poison,
	revive
};

struct SItem
{
	string name;
	EEffect effect;
	TInt32 value;
};

const SItem POTION = { "Potion", restoreHealth, 20 };
const SItem SUPER_POTION = { "Potion", restoreHealth, 50 };

const SItem MAGIC_POTION = { "Magic Potion", restoreMana, 40 };
const SItem SUPER_MAGIC_POTION = { "Super Magic Potion", restoreMana, 70 };

const SItem VENOM = { "Venom", poison, 10 };

const SItem REVIVE = { "Revive", revive, 50 };
}