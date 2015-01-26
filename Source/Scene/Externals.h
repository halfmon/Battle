#pragma once

#include <vector>
#include "Defines.h"
#include "CVector3.h"
#include "EntityManager.h"

namespace gen
{
	extern CEntityManager EntityManager;
	extern CMessenger Messenger;

	extern TInt32 NumTotal;
	extern vector<TEntityUID> AttackOrder;

	extern vector<CAttack> ListOfAttacks;
	extern vector<CItem> ListOfItems;
	extern vector<CDefence> ListOfDefence;

	extern CAttackEffect attackEffect;
	extern CItemEffect itemEffect;

	extern TEntityUID RandomEnemy();
	extern TEntityUID RandomAlly();
	extern bool EnemyAlive();
	extern bool AllyAlive();

	extern TUInt32 ViewportWidth;
	extern TUInt32 ViewportHeight;
	extern CCamera* MainCamera;

	extern vector<TEntityUID> Allies;
	extern vector<TEntityUID> Enemies;

	extern int generalAI;
	extern bool effectOn;
	extern bool templateAIOn;

	extern TEntityUID LowestHealthAlly();
	extern TEntityUID LowestHealthEnemy();

	extern TEntityUID LowestMagicAlly();
	extern TEntityUID LowestMagicEnemy();

	extern TEntityUID DeadEnemy();
	extern TEntityUID DeadAlly();
}