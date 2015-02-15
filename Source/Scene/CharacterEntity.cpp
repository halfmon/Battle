/************************************************
	CharacterEntity.cpp

	Character entity template and entity classes
*************************************************/

#include <vector>

#include "CharacterEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
//#include "AttackEffect.h"
//#include "ItemEffect.h"
#include "Externals.h"

namespace gen
{

/* Picks the best attack based only on the damage value of the attack. If two attacks could kill the target based on the attacks damage it will choose the 
attack that cost the least. */
TInt32 PickBestAttack( CCharTemplate* attackList, TInt32 targetHealth, TInt32 attackerMagic )
{
	CAttack preAttack = attackList->GetAttack(0);
	int preAttackNum = 0;
	CAttack bestAttack = attackList->GetAttack(0);
	int attackNum = 0;
	for ( int i = 1; i < attackList->GetAttackNum(); i++ )
	{
		CAttack currentAttack = attackList->GetAttack(i);
		if ( currentAttack.GetType() == Magical )
		{
			if ( attackerMagic - currentAttack.GetCost() >= 0 )
			{
				if ( bestAttack.GetCost() > currentAttack.GetCost() && targetHealth - bestAttack.GetDamage() <= 0 && targetHealth - currentAttack.GetDamage() <= 0 )
				{
					preAttack = bestAttack;
					preAttackNum = attackNum;
					bestAttack = currentAttack;
					attackNum = i;
				}
				else if ( bestAttack.GetDamage() < currentAttack.GetDamage() )
				{	
					preAttack = bestAttack;
					preAttackNum = attackNum;
					bestAttack = currentAttack;
					attackNum = i;
				}
			}
		}
		else
		{
			if ( bestAttack.GetDamage() >  currentAttack.GetDamage() && targetHealth - bestAttack.GetDamage() <= 0 && targetHealth -  currentAttack.GetDamage() <= 0 )
			{
				preAttack = bestAttack;
				preAttackNum = attackNum;
				bestAttack = currentAttack;
				attackNum = i;
			}
			else if ( bestAttack.GetDamage() <  currentAttack.GetDamage() )
			{
				preAttack = bestAttack;
				preAttackNum = attackNum;
				bestAttack = currentAttack;
				attackNum = i;
			}
		}
	}
	return attackNum;
}

vector<EAttackElement> getattacklist( std::string name )
{
	vector<EAttackElement> List;
	if( name == "enemy" )
	{
		for( auto listIt = Allies.begin(); listIt != Allies.end(); listIt++ )
		{
			CCharEntity* current = static_cast<CCharEntity*>(EntityManager.GetEntity( *listIt ));
			for( int i = 0; i < current->GetTemplate()->GetAttackNum(); i++ )
			{
				List.push_back( current->GetTemplate()->GetAttack( i ).GetElement() );
			}
		}
	}
	else
	{
		for( auto listIt = Enemies.begin(); listIt != Enemies.end(); listIt++ )
		{
			CCharEntity* current = static_cast<CCharEntity*>(EntityManager.GetEntity( *listIt ));
			for( int i = 0; i < current->GetTemplate()->GetAttackNum(); i++ )
			{
				List.push_back( current->GetTemplate()->GetAttack( i ).GetElement() );
			}
		}
	}
	return List;
}
/*-----------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------
	Car Entity Class
-------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------*/

CCharEntity::CCharEntity
(
	CCharTemplate*  charTemplate,
	TEntityUID      UID,
	const string&   name,
	const CVector3& position,
	const CVector3& rotation,
	const CVector3& scale
) : CEntity( charTemplate, UID, name, position, rotation, scale )
{
	m_CharTemplate = charTemplate;

	m_State = Wait;
	m_CurrentHealth = charTemplate->GetMaxHealth();
	m_CurrentMagic  = charTemplate->GetMaxMagic();
	m_CurrentAttack = -1;
	m_CurrentDefence = -1;
	m_Defend = false;
	m_DefendLast = false;
	m_WeaknessList.push_back(charTemplate->GetWeakness());
}

// Used to get the name of the last attack the character used.
std::string CCharEntity::GetAttackName()
{
	if ( m_CurrentAttack == -1 )
	{
		return "???";
	}
	else
	{
		return m_CharTemplate->GetAttack(m_CurrentAttack).GetName();
	}
	return "???";
}
// Used to get a string for the output of whether or not the character is defending.
std::string CCharEntity::GetDefenceInfo()
{
	if ( m_CurrentDefence == -1 )
	{
		return "Not Defending";
	}
	else
	{
		return "Defending";
	}
}

// The character picks a random target from the other team and attacks them with a random attack that they can use.
// Will keep repicking the attack if they do not have the magic to use the chosen attack.
void CCharEntity::RandomAttack( SMessage msg )
{
	// Choosing Attack: Random pick an attack, check if the character has the magic to use the attack and if they do they will choose that attack.
	m_CurrentAttack = Random(0, m_CharTemplate->GetAttackNum()-1);
	if ( m_CharTemplate->GetAttack(m_CurrentAttack).GetType() == Magical )
	{
		if ( m_CurrentMagic - m_CharTemplate->GetAttack(m_CurrentAttack).GetCost() >= 0 )
		{
			m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).GetCost();
		}
		else
		{
			do
			{
				m_CurrentAttack = Random(0, m_CharTemplate->GetAttackNum() - 1);
			} while ( m_CurrentMagic - m_CharTemplate->GetAttack(m_CurrentAttack).GetCost() < 0 );
		}
	}

	// Choosing Target: Randomly pick a target from the other team.
	TEntityUID target;
	if( m_CharTemplate->GetType() == "enemy" )
	{
		target = RandomAlly();
	}
	else
	{
		target = RandomEnemy();
	}

	// Setting up the attack message.
	msg.type = Msg_Attacked;
	msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack).Attack( GetUID() );
	if(msg.attack.type == Physical)
	{
		msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f); //Multiplies the attack damage by the strength of the character.
	}
	else
	{
		msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f); //Multiplies the attack damage by the Intelligence of the character.
	}
	msg.from = GetUID();
	if(effectOn)
	{
		attackEffect.StartAttack(Matrix().GetPosition(), target, msg);
	}
	else
	{
		Messenger.SendMessage( target, msg );
	}
	m_State = Wait;
}
// Picks a target that has the a weakness to an attack the character has, and will attack with the strongest attack they have that the target is weak too. 
void CCharEntity::WeaknessAttack( SMessage msg )
{
	// Checks the charactesr current health and if it is lower then 25% of their max health decides if it will defend or not.
	// The character picks a random number between 0 and 10 and if it is less then 7 they will pick a random one of their defence types and use it if they
	// have enough magic.
	if ( m_CurrentHealth < m_CharTemplate->GetMaxHealth() / 4 )
	{
		int chance = Random( 0, 10 );
		if ( chance < 7 )
		{
			int cost;
			do
			{
				chance = Random( 0, m_CharTemplate->GetDefenceNum()-1);
				cost = m_CharTemplate->GetDefence(chance).GetCost();
			} while ( m_CurrentMagic - cost < 0 );
			m_CurrentDefence = chance;
			m_Defend = true;
			m_CurrentMagic -= cost;
		}
	}
	// Makeing sure that they are not both defening and using an item in the same turn.
	bool useItem;
	if ( m_Defend )
	{
		useItem = false;
	}
	else
	{
		useItem = UseItem(msg);
	}

	if(!m_Defend && !useItem)
	{
		TEntityUID target;
		int attackTries = 5; // Number of times that the character will loop around for the recoil and additional weaknesses.
		int numAddedWeaknessAttacks = 0; // The number of attacks that the character will become weak to from the attack.
		int recoilAfterMath = 100; // The health that the character will have after the the recoil damge from the attack.

		if(Template()->GetType() == "enemy")
		{
			bool weakness = false;
			for(auto it = Allies.begin(); it != Allies.end(); it++) // Runs through all of the entities in the allies list.
			{
				for ( int i = 0; i < static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetNumWeakness(); i++ ) // Runs through all of the weakness for each entity.
				{
					EAttackElement targetWeakness = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetWeakness( i ); // Get the current weakness.
					bool targetDead = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead();                       // Check if the target is dead.
					for(int j = 0; j < m_CharTemplate->GetAttackNum(); j++)                                                    // Run through all of the attacks the character has.
					{
						if(targetWeakness == m_CharTemplate->GetAttack(j).GetElement() && !targetDead)                         // Check if attack is the same element as the defence and that the target is not dead.
						{
							weakness = true;
							target = *it;
						}
					}
				}
			}
			if(!weakness ) //If there is no target with a weakness to an attack the character has then it will pick the one with the lowest current health.
			{
				target = LowestHealthAlly();
			}
		}
		else // Sames as above but for the other team.
		{
			bool weakness = false;
			for(auto it = Enemies.begin(); it != Enemies.end(); it++)
			{
				for ( int i = 0; i < static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetNumWeakness(); i++ )
				{
					EAttackElement targetWeakness = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetWeakness( i );
					bool targetDead = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead();
					for(int j = 0; j < m_CharTemplate->GetAttackNum(); j++)
					{
						if(targetWeakness == m_CharTemplate->GetAttack(j).GetElement() && !targetDead)
						{
							weakness = true;
							target = *it;
						}
					}
				}
			}
			if(!weakness)
			{
				target = LowestHealthEnemy();
			}
		}

		do
		{
			int targetHealth = static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth(); // Get the targets current health.
			bool foeWeakness = false; // Used to deside which attack to use based on if the target has a weakness
			int weaknessAttack = 0;

			for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
			{
				// For each attack attack check if the target has a weakness to the attack and set that as the weakness attack to use.
				for ( int j = 0; j < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetNumWeakness(); j++ ) // Runs through all of the weakness for each entity.
				{
					if(static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetWeakness(j) == m_CharTemplate->GetAttack(i).GetElement())
					{
						weaknessAttack = i;
						foeWeakness = true;
					}
				}
			}

			// If the character has an attack that the target is weak to it checks that it has enough magic to use the attack.
			if(foeWeakness && m_CurrentMagic - m_CharTemplate->GetAttack(weaknessAttack).GetCost() > 0)
			{
				m_CurrentAttack = weaknessAttack;
			}
			else // If the character cannot use an attack that the target is weak to then they will use the strongest attack that they can.
			{
				m_CurrentAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic);
			}

			recoilAfterMath = m_CharTemplate->GetAttack(m_CurrentAttack).RecoilCalculation( m_CurrentHealth ); //Get the health left after the recoil from the attack.
			int numEnemyAttacks = m_CharTemplate->GetAttack(m_CurrentAttack).WeaknesHasEffect(getattacklist(m_CharTemplate->GetType())); // Get the number of attacks that the character will become weak to.
			numAddedWeaknessAttacks = Random(0,numEnemyAttacks); // Get a random number based on the number of attacks the character will become weak to. The more attacks the less likely to choose that attack.
			attackTries--; // Decrease the number of tries for picking an attack.

		}while( numAddedWeaknessAttacks != 0 && attackTries > 0 && recoilAfterMath <= 0 );
		m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).GetCost(); // Decrease current magic by the cost of the attack.

		// Setting up the attack message.
		msg.type = Msg_Attacked;
		msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack).Attack( GetUID() );
		if(msg.attack.type == Physical)
		{
			msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f);
		}
		else
		{
			msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f);
		}
		msg.from = GetUID();
		if(effectOn)
		{
			attackEffect.StartAttack(Matrix().GetPosition(), target, msg);
		}
		else
		{
			Messenger.SendMessage(target,msg);
		}
		if(m_Poison == PoisonedWeapon)
		{
			m_Poison = None;
			msg.type = Msg_Poison;
			Messenger.SendMessage(target,msg);
		}
	}
	m_State = Wait;
}
// Attacks the target with the lowest current health, with the strongest attack that they have.
void CCharEntity::StrongestAttack( SMessage msg )
{
	//Check to see if the character can use an item.
	if(!UseItem(msg))
	{
		std::string type = m_CharTemplate->GetType();
		// Checks the charactesr current health and if it is lower then 25% of their max health decides if it will defend or not.
		// The character picks a random number between 0 and 10 and if it is less then 7 will decide what defence to used based on the stats of the other team.
		if(m_CurrentHealth < m_CharTemplate->GetMaxHealth() / 4)
		{
			int chance = Random(0,10);
			if(chance < 7 && !m_DefendLast)
			{
				int averageStrength = 0;
				int averageIntelligence = 0;

				if(type == "enemy")
				{
					// Gets the average strength and intelligence of the other side.
					for(auto it = Allies.begin(); it != Allies.end(); it++)
					{
						averageStrength += static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetStrength();
					}
					averageStrength /= Allies.size();
					for(auto it = Allies.begin(); it != Allies.end(); it++)
					{
						averageIntelligence += static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetIntelligence();
					}
					averageIntelligence /= Allies.size();
				}
				else
				{
					// Gets the average strength and intelligence of the other side.
					for(auto it = Enemies.begin(); it != Enemies.end(); it++)
					{
						averageStrength += static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetStrength();
					}
					averageStrength /= Enemies.size();
					for(auto it = Enemies.begin(); it != Enemies.end(); it++)
					{
						averageIntelligence += static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetIntelligence();
					}
					averageIntelligence /= Enemies.size();
				}
				// If the averages strength of the other team is greater then the average intelligence then the character will pick a defence that can be used against physical attacks as they are more
				// likely to be attacked with a physical attack. if the average intelligence is greater then it will pick a defence that can be used against magical attacks.
				if(averageStrength > averageIntelligence)
				{
					do
					{
						m_CurrentDefence = Random(0,m_CharTemplate->GetDefenceNum()-1);
					} while (m_CurrentMagic - m_CharTemplate->GetDefence(m_CurrentDefence).GetCost() < 0 && ( m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Physical
						|| m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Both ) );
				}
				else
				{
					do
					{
						m_CurrentDefence = Random(0,m_CharTemplate->GetDefenceNum()-1);
					} while (m_CurrentMagic - m_CharTemplate->GetDefence(m_CurrentDefence).GetCost() < 0 && m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Magical
						&& m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Both);
				}
				m_CurrentMagic -= m_CharTemplate->GetDefence(m_CurrentDefence).GetCost();

				m_Defend = true;
				m_DefendLast = true;
			}
			else
			{
				m_DefendLast = false;
			}
		}
		// If the character is not defending then they will attack.
		if(!m_Defend)
		{
			TEntityUID target;

			int attackTries = 5; // Number of times that the character will loop around for the recoil and additional weaknesses.
			int numAddedWeaknessAttacks = 0; // The number of attacks that the character will become weak to from the attack.
			int recoilAfterMath = 100; // The health that the character will have after the the recoil damge from the attack.

			// Picking a target with the lowest health.
			if(type == "enemy")
			{
				target = LowestHealthAlly();
			}
			else
			{
				target = LowestHealthEnemy();
			}

			do
			{
				int targetHealth = static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth(); // Get the targets health.
				bool targetWeakness = false;
				int weaknessAttack = 0;
				for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
				{
					// For each attack attack check if the target has a weakness to the attack and set that as the weakness attack to use.
					for ( int j = 0; j < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetNumWeakness(); j++ ) // Runs through all of the weakness for each entity.
					{
						if(static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetWeakness(j) == m_CharTemplate->GetAttack(i).GetElement())
						{
							weaknessAttack = i;
							targetWeakness = true;
						}
					}
				}
				int bestAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic); // Choose the strongest attack that the user has based on damage before stat modification.

				if(bestAttack != weaknessAttack && targetWeakness) 
				{
					CAttack weaknessA = m_CharTemplate->GetAttack(weaknessAttack); 
					CAttack bestA = m_CharTemplate->GetAttack(bestAttack);
	
					// Find out how much damage the attack the target is weak to and the strongest attack the character can use will do after the stat modification.
					float weaknessDamage = weaknessA.GetDamage() * 2;
					float bestDamage = bestA.GetDamage();
					if(weaknessA.GetType() == Physical)
					{
						weaknessDamage = weaknessDamage * m_CharTemplate->GetStrength() / 100.0f + 0.5f;
					}
					else
					{
						weaknessDamage = weaknessDamage * m_CharTemplate->GetIntelligence() / 100.0f + 0.5f;
					}
					if(bestA.GetType() == Physical)
					{
						bestDamage = bestDamage * m_CharTemplate->GetStrength() / 100.0f + 0.5f;
					}
					else
					{
						bestDamage = bestDamage * m_CharTemplate->GetIntelligence() / 100.0f + 0.5f;
					}
					if( weaknessDamage > bestDamage )
					{
						m_CurrentAttack = weaknessAttack;
					}
					else
					{
						m_CurrentAttack = bestAttack;
					}
				}
				else
				{
					m_CurrentAttack = bestAttack;
				}
	
				recoilAfterMath = m_CharTemplate->GetAttack(m_CurrentAttack).RecoilCalculation( m_CurrentHealth ); //Get the health left after the recoil from the attack.
				int numEnemyAttacks = m_CharTemplate->GetAttack(m_CurrentAttack).WeaknesHasEffect(getattacklist(m_CharTemplate->GetType())); // Get the number of attacks that the character will become weak to.
				numAddedWeaknessAttacks = Random(0,numEnemyAttacks); // Get a random number based on the number of attacks the character will become weak to. The more attacks the less likely to choose that attack.
				attackTries--; // Decrease the number of tries for picking an attack.

			}while( numAddedWeaknessAttacks != 0 && attackTries > 0 && recoilAfterMath <= 0 ); //Makes sure that condition are met for doing the least harm to the character.
			m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).GetCost();

			//Setting up the attack message.
			msg.type = Msg_Attacked;
			msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack).Attack( GetUID() );
			if(msg.attack.type == Physical)
			{
				msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f);
			}
			else
			{
				msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).GetDamage() * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f);
			}
			msg.from = GetUID();

			if(effectOn)
			{
				attackEffect.StartAttack( Matrix().GetPosition(), target, msg);
			}
			else
			{
				Messenger.SendMessage(target,msg);
			}
		}
	}
	m_State = Wait;
}

// Runs through the rules for taking damage.
// The cost of defence is decided when it is chosen so the check is not needed for the cost.
void CCharEntity::TakingDamage( SMessage msg )
{
	float defence = 1.0f;  //A modifier for the damage taken by the character.
	// Going throught the types of defence and using the correct rules if the are defending.
	// Does not go through if the type of damage that they are recieving damage from recoil.
	if ( m_Defend && msg.attack.type != Recoil )
	{
		CDefence pick = m_CharTemplate->GetDefence(m_CurrentDefence);
		// Regular: Makes sure that the defence type and element match the attack and change defence modifier if they do.
		if (pick.GetType() == Regular)
		{
			if (pick.GetAttackRecivedType() == msg.attack.type || pick.GetAttackRecivedType() == Both)
			{
				if ( pick.GetElement() == None || pick.GetElement() == msg.attack.element )
				{
						defence = pick.GetModifier();
				}
			}
		}
		// Reflect: Sets defence modifier to 0 so the user takes no damage, and sends a message to the attacker that they are taking the damage.
		else if (pick.GetType() == Reflect)
		{
				defence = 0.0f;
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.GetModifier();
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
		}
		// Pain Split: Sends a damage message to the attacker that is half of the damage of the attack.
		else if (pick.GetType() == PainSplit)
		{
				defence = pick.GetModifier();
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.GetModifier();
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
		}

		m_Defend = false;
		m_CurrentDefence = -1;
	}
	// Sets up extra damage done because of weaknesses. weaknessDamage is set to the negative of the attack damage so that it will equal 0 when added to the base damage
	// if the character is not weak to the attack.
	TFloat32 weaknessDamage = 0;
	for( auto it = m_WeaknessList.begin(); it != m_WeaknessList.end(); it++ )
	{
		if ( msg.attack.element == (*it).element )
		{
			weaknessDamage = msg.attack.damage * (*it).modifier;
		}
	}
	m_CurrentHealth -= static_cast<TInt32>( (msg.attack.damage + weaknessDamage) * defence );
}

//The implementation of the update function for the character entities.
bool CCharEntity::Update( TFloat32 updateTime )
{
	//Checking the messages that are recived and decides what to do based on the mmessage recived.
	SMessage msg;
	while (Messenger.FetchMessage( GetUID(), &msg ))
	{
		switch (msg.type)
		{
			case Msg_Stop:
				m_State = Wait;
				break;
			case Msg_Act:
				if (m_State != Dead)
				{
					m_State = Act;
					m_Defend = false;
				}
				break;
			case Msg_Attacked:
				if ( m_State != Dead )
				{
					TakingDamage( msg );
				}
				break;
			case Msg_HealthRestored: // Restored health and sets it to the max if it excides the characters max.
				m_CurrentHealth += msg.item.value;
				if ( m_CurrentHealth > m_CharTemplate->GetMaxHealth() )
				{
					m_CurrentHealth = m_CharTemplate->GetMaxHealth();
				}
				break;
			case Msg_MagicRestored: // Restored magic and sets it to the max if it excides the characters max.
				m_CurrentMagic += msg.item.value;
				if(m_CurrentMagic > m_CharTemplate->GetMaxMagic())
				{
					m_CurrentMagic = m_CharTemplate->GetMaxMagic();
				}
				break;
			case Msg_Revive:  // Brings the target back from the dead with half of its maximum health.
				if (m_State == Dead)
				{
					Matrix().SetY(0.0f);
					m_State = Wait;
					m_CurrentHealth = m_CharTemplate->GetMaxHealth() / 2;
				}
				break;
			case Msg_Poison: // Sets up the target to be poisoned.
				m_Poison = IsPoisoned;
				m_PoisonCount = 5;
				break;
		}
	}

	// Puts the character into the dead state if they have no health and is not dead.
	if ( m_CurrentHealth <= 0 && m_State != Dead )
	{
		Matrix().SetY(-10.0f);
		m_State = Dead;
	}

	//Used to clean up the inventory if they have none of a certain type of item by removing the empty reference to it.
	if( m_Inventory.size() > 0 )
	{
		vector<int> none;
		for(int i = 0; i < static_cast<int>(m_Inventory.size()); i++)
		{
			if(m_Inventory[i].Quantity <= 0)
			{
				none.push_back(i);
			}
		}

		if(none.size() == m_Inventory.size())
		{
			m_Inventory.clear();
		}
		else
		{
			for(int i = 0; i < static_cast<int>(none.size()); i++)
			{
				m_Inventory.erase(m_Inventory.begin() + none[i]);
			}
		}
	}

	if (m_State == Act)
	{
		//Decides whether or not they attack based one whether there are any targets left alive.
		bool attack = false;
		if( Template()->GetType() == "ally" )
		{
			 attack = EnemyAlive();
		}
		else
		{
			 attack = AllyAlive();
		}
		if ( attack )
		{
			//Sets the variable for which AI is used depending on whether or not the template AI is being used.
			int AIUsed;
			if(templateAIOn)
			{
				AIUsed = m_CharTemplate->GetAI();
			}
			else
			{
				AIUsed = generalAI;
			}

			// Picking which AI to use.
			if ( AIUsed == 1 )
			{
				RandomAttack( msg );
			}
			if ( AIUsed == 2 )
			{
				WeaknessAttack( msg );
			}
			if ( AIUsed == 3 )
			{
				StrongestAttack( msg );
			}

			numTurns++; // Increase the count for the number of turns that have passed.

			//Apply the poison damage if poisoned and removes it if the countdown has finished.
			if(m_Poison == IsPoisoned)
			{
				m_PoisonCount--;
				m_CurrentHealth = static_cast<TInt32>(m_CurrentHealth * 0.9f);
				if(m_PoisonCount < 1)
				{
					m_Poison = None;
				}
			}
		}

		//Checking to see any limited time weaknesses are ready to be removed and decreaments the one that are not. Perminent weaknesses are set to below -50 so they don't get removed.
		if( m_WeaknessList.size() > 1 )
		{
			vector<int> remove;
			for( auto i = 0; i < m_WeaknessList.size(); i++ )
			{
				if ( m_WeaknessList[i].turns > -50 )
				{
					m_WeaknessList[i].turns--;
					if ( m_WeaknessList[i].turns <= 0 )
					{
						remove.push_back( i );
					}
				}
			}
			for( int i = 0; i < remove.size(); i++ )
			{
				m_WeaknessList.erase( m_WeaknessList.begin() + remove[i] );
			}
		}

	}

	if(m_State == Dead)
	{
	}

	return true;
}

// Takes an item class and add that item to the characters invantory.
void CCharEntity::AddItemToInvantory( CItem newItem )
{
	bool haveOne = false;
	int haveOnePos = 0;
	for(int i = 0; i < static_cast<int>(m_Inventory.size()); i++)
	{
		if ( m_Inventory[i].item.GetName() == newItem.GetName() )
		{
			haveOne = true;
		}
	}
	if ( haveOne )
	{
		m_Inventory[haveOnePos].Quantity++;
	}
	else
	{
		SInventoryItem  newEntry = { newItem, 1 };
		m_Inventory.push_back( newEntry );
	}
}

// Lets the character us an item in it's invantory. Returns true if an item has been used and false if no items has been used.
bool CCharEntity::UseItem( SMessage msg )
{
	// If no items in invantory, quits straight away.
	if ( m_Inventory.size() == 0 )
	{
		return false;
	}
	else
	{
		for(int i = 0; i < static_cast<int>(m_Inventory.size()); i++)
		{
			// Finds the character on the same team that has the lowest health, checks if it is less they 10% if its maximum health.
			// If it is less then 10% then uses a restore health message and and deceases the item by 1.
			if ( m_Inventory[i].item.GetEffect() == restoreHealth )
			{
				TEntityUID current;
				CCharEntity* hurt;
				if ( m_CharTemplate->GetType() == "ally" )
				{
					current = LowestHealthAlly();
					hurt = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}
				else
				{
					current = LowestHealthEnemy();
					hurt = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}

				if ( hurt->GetCurrentHealth() < (hurt->m_CharTemplate->GetMaxHealth() * 0.3) )
				{
					msg.type = Msg_HealthRestored;
					msg.from = GetUID();
					msg.item = m_Inventory[i].item.Use(GetUID());
					if(effectOn)
					{
						itemEffect.StartEffect(Position(),current,msg);
					}
					else
					{
						Messenger.SendMessage(current,msg);
					}
					m_Inventory[i].Quantity--;
					return true;
				}

			}
			// Finds the character on the same team that has the lowest magic, checks if it is less they 10% if its maximum magic.
			// If it is less then 10% then uses a restore health message and and deceases the item by 1.
			else if ( m_Inventory[i].item.GetEffect() == restoreMana )
			{
				TEntityUID current;
				CCharEntity* lowMagic;
				if ( m_CharTemplate->GetType() == "ally" )
				{
					current = LowestMagicAlly();
					lowMagic = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}
				else
				{
					current = LowestMagicEnemy();
					lowMagic = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}

				if ( lowMagic->GetCurrentMagic() < (lowMagic->m_CharTemplate->GetMaxMagic() * 0.3) )
				{
					msg.type = Msg_MagicRestored;
					msg.from = GetUID();
					msg.item = m_Inventory[i].item.Use(GetUID());
					if(effectOn)
					{
						itemEffect.StartEffect(Position(),current,msg);
					}
					else
					{
						Messenger.SendMessage(current,msg);
					}
					m_Inventory[i].Quantity--;
					return true;
				}
			}
			// Checks to see if a character on the same team is dead and then sends a revive message to the dead character and decrease item by 1.
			else if ( m_Inventory[i].item.GetEffect() == revive )
			{
				TEntityUID target;
				if ( m_CharTemplate->GetType() == "enemy" )
				{
					target = DeadEnemy();
				}
				else
				{
					target = DeadAlly();
				}
				
				if ( target != NULL )
				{
					msg.type = Msg_Revive;
					msg.from = GetUID();
					msg.item = m_Inventory[i].item.Use(GetUID());
					if(effectOn)
					{
						itemEffect.StartEffect(Position(),target,msg);
					}
					else
					{
						Messenger.SendMessage(target,msg);
					}
					m_Inventory[i].Quantity--;
					return true;
				}
			}
			// Checks to see if the character has not already used posion, and if it has not then it will use poison of the weapon.
			else if ( m_Inventory[i].item.GetEffect() == poison )
			{
				if ( m_Poison != PoisonedWeapon )
				{
					if(effectOn)
					{
						itemEffect.StartEffect(Position(),GetUID(),msg);
					}
					m_Poison = PoisonedWeapon;
					m_Inventory[i].Quantity--;
					return true;
				}
			}
		}
		return false;
	}
	return false;
}

} //namespace gen