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
#include "ItemEffect.h"

namespace gen
{
const TInt32 NumInvantory = 5;

/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Character Template Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

class CCharTemplate: public CEntityTemplate
{
public:
	CCharTemplate
		(
		const string& type,const string& name,const string& meshFilename,
		TInt32 maxHealth,TInt32 maxMagic,TInt32 st,TInt32 in,TInt32 sp,
		vector<CAttack> attacks,vector<CDefence> defences,EAttackElement weak,TInt32 AI
		): CEntityTemplate(type,name,meshFilename)
	{
		m_MaxHealth = maxHealth;
		m_MaxMagic = maxMagic;

		m_Stats.strength = st;
		m_Stats.intelligence = in;
		m_Stats.speed = sp;

		m_BaseWeakness.element = weak;
		m_BaseWeakness.modifier = 1.0f;
		m_BaseWeakness.turns = -100;

		m_AI = AI;

		for(int i = 0; i < static_cast<int>(attacks.size()); i++)
		{
			m_Attacks.push_back(attacks[i]);
		}
		for (int i = 0; i < static_cast<int>(defences.size()); i++)
		{
			m_Defences.push_back(defences[i]);
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
	vector<CAttack> m_Attacks;
	vector<CDefence> m_Defences;
	SWeakness m_BaseWeakness;
	TInt32 m_AI;


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

	CAttack GetAttack(int i)
	{
		return m_Attacks[i];
	}
	CDefence GetDefence(int i)
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

	SWeakness GetWeakness()
	{
		return m_BaseWeakness;
	};
	TInt32 GetAI()
	{
		return m_AI;
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
		CItem item;
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
	vector<SWeakness> m_WeaknessList;

	bool m_DefendLast; // A bool to decide if the character defended last turn and alter the chance of defending for the next turn

	TInt32 m_CurrentAttack;
	TInt32 m_CurrentDefence;

/////////////////////////////////////
//	Public interface
public:
	//The implementation of the update function for the character entities.
	virtual bool Update(TFloat32 updateTime);
	// The character picks a random target from the other team and attacks them with a random attack that they can use.
	// Will keep repicking the attack if they do not have the magic to use the chosen attack.
	virtual void RandomAttack( SMessage );
	// Picks a target that has the a weakness to an attack the character has, and will attack with the strongest attack they have that the target is weak too. 
	virtual void WeaknessAttack( SMessage );
	// Attacks the target with the lowest current health, with the strongest attack that they have.
	virtual void StrongestAttack( SMessage );
	// Runs through the rules for taking damage.
	// The cost of defence is decided when it is chosen so the check is not needed for the cost.
	virtual void TakingDamage( SMessage );

	// Takes an item class and add that item to the characters invantory.
	virtual void AddItemToInvantory( CItem );
	// Lets the character us an item in it's invantory. Returns true if an item has been used and false if no items has been used.
	virtual bool UseItem( SMessage );

	// Used to get the name of the last attack the character used.
	string GetAttackName();
	// Used to get a string for the output of whether or not the character is defending.
	string GetDefenceInfo();

	// Takes a new weakness and adds it to the list of weaknesses.
	void AddWeakness(SWeakness newWeakness)
	{
		m_WeaknessList.push_back(newWeakness);
	}

	// Returns true if the character is dead and false if they are alive.
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

	////////////////////
	//     Getters
	TInt32 GetCurrentHealth()
	{
		return m_CurrentHealth;
	}
	TInt32 GetCurrentMagic()
	{
		return m_CurrentMagic;
	}
	CCharTemplate* GetTemplate()
	{
		return m_CharTemplate;
	}
	TInt32 GetNumWeakness()
	{
		return static_cast<TInt32>( m_WeaknessList.size() );
	}
	EAttackElement GetWeakness( TInt32 pick ) // Returns the element of the weakness at that point in the list.
	{
		return m_WeaknessList[pick].element;
	}
};

} // namespace gen