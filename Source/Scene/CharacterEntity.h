/************************************************
	CharacterEntity.h

	Character entity template and entity classes
*************************************************/

#pragma once

#include <string>
#include <vector>
using namespace std;

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"
#include "Messenger.h"
#include "Attacks.h"
#include "Defence.h"
#include "Items.h"
#include "AttackEffect.h"

namespace gen
{
const TInt32 NumAttacks = 4;
const TInt32 NumDefences = 4;
const TInt32 NumInvantory = 5;

/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Character Template Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

class CCharTemplate : public CEntityTemplate
{
public:
	CCharTemplate
	(
		const string& type, const string& name, const string& meshFilename,
		TInt32 maxHealth, TInt32 maxMagic, TInt32 st, TInt32 in, TInt32 sp,
		vector<SAttack> attacks, vector<SDefence> defences, EElement weak
	) : CEntityTemplate( type, name, meshFilename )
	{
		m_MaxHealth = maxHealth;
		m_MaxMagic = maxMagic;
		m_Stats.strength = st;
		m_Stats.intelligence = in;
		m_Stats.speed = sp;
		m_Weakness = weak;

		for (int i = 0; i < NumAttacks; i++)
		{
			m_Attacks.push_back( attacks[i] );
		}
		for (int i = 0; i < NumDefences; i++)
		{
			m_Defences.push_back( defences[i] );
		}
	}

/////////////////////////////////////
//	Private interface
private:
	TInt32 m_MaxHealth;
	TInt32 m_MaxMagic;

	struct SStats
	{
		TInt32 strength;
		TInt32 intelligence;
		TInt32 speed;
	};

	SStats  m_Stats;
	vector<SAttack> m_Attacks;
	vector<SDefence> m_Defences;
	EElement m_Weakness;


/////////////////////////////////////
//	Public interface
public:

	/////////////////////////////////////
	//	Getters

	TInt32 GetMaxHealth()
	{
		return m_MaxHealth;
	}
	TInt32 GetMaxMagic()
	{
		return m_MaxMagic;
	}

	TInt32 GetStrength()
	{
		return m_Stats.strength;
	}
	TInt32 GetIntelligence()
	{
		return m_Stats.intelligence;
	}
	TInt32 GetSpeed()
	{
		return m_Stats.speed;
	}
	SAttack GetAttack( int i )
	{
		return m_Attacks[i];
	}
	SDefence GetDefence( int i )
	{
		return m_Defences[i];
	}
	int GetAttackNum()
	{
		return m_Attacks.size();
	}
	int GetDefenceNum()
	{
		return m_Defences.size();
	}
	EElement GetWeakness()
	{
		return m_Weakness;
	}
};

class CCharEntity : public CEntity
{
public:
	CCharEntity
		(
		CCharTemplate*  charTemplate,
		TEntityUID      UID,
		const string&   name = "",
		const CVector3& position = CVector3::kOrigin,
		const CVector3& rotation = CVector3(0.0f, 0.0f, 0.0f),
		const CVector3& scale = CVector3(1.0f, 1.0f, 1.0f)
		);

/////////////////////////////////////
//	Private interface
private:
	enum EState
	{
		Act,
		Wait,
		Dead
	};
	enum EPoisonState
	{
		IsPoisoned,
		PoisonedWeapon,
		None
	};

	struct SInventoryItem
	{
		SItem item;
		TInt32 Quantity;
	};
	/////////////////////////////////////
	// Data
	CCharTemplate* m_CharTemplate;

	TInt32 m_CurrentHealth;
	TInt32 m_CurrentMagic;

	bool m_Defend;
	EState m_State;
	EPoisonState m_Poison;
	int m_PoisonCount;

	vector<SInventoryItem> m_Inventory;

	bool m_DefendLast; // A bool to decide if the character defended last turn and alter the chance of defending for the next turn

	TInt32 m_CurrentAttack;
	TInt32 m_CurrentDefence;

/////////////////////////////////////
//	Public interface
public:
	virtual bool Update(TFloat32 updateTime);
	virtual void RandomAttack( SMessage );
	virtual void ReacativeAttack( SMessage );
	virtual void Planning( SMessage );
	virtual void TakingDamage( SMessage );

	virtual void AddItemToInvantory( SItem );
	virtual bool UseItem( SMessage );

	TInt32 GetCurrentHealth()
	{
		return m_CurrentHealth;
	}
	TInt32 GetCurrentMagic()
	{
		return m_CurrentMagic;
	}
	TInt32 GetNumInInvantory();
	string GetAttackElement();
	string GetDefenceInfo();

	CCharTemplate* GetTemplate()
	{
		return m_CharTemplate;
	}

	// Returns true if the character is dead and false if they are alive
	bool isDead()
	{
		if ( m_State != Dead )
		{
			return false;
		}
		else
		{
			return true;
		}
	}
};

} // namespace gen