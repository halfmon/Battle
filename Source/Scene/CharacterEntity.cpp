/************************************************
	CharacterEntity.cpp

	Character entity template and entity classes
*************************************************/

#include "CharacterEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "AttackEffect.h"
#include "Externals.h"

namespace gen
{

/* Picks the best attack based only on the damage value of the attack. If two attacks could kill the target based on the attacks damage it will choose the 
attack that cost the least if it is magical. */
TInt32 PickBestAttack( CCharTemplate* attackList, TInt32 targetHealth, TInt32 attackerMagic )
{
	SAttack bestAttack = attackList->GetAttack(0);
	int attackNum = 0;
	for ( int i = 1; i < attackList->GetAttackNum(); i++ )
	{
		SAttack currentAttack = attackList->GetAttack(i);
		if ( currentAttack.type == Magical )
		{
			if ( attackerMagic - currentAttack.cost >= 0 )
			{
				if ( bestAttack.cost > currentAttack.cost && targetHealth - bestAttack.damage <= 0 && targetHealth - currentAttack.damage <= 0 )
				{
					bestAttack = currentAttack;
					attackNum = i;
				}
				else if ( bestAttack.damage < currentAttack.damage )
				{			
					bestAttack = currentAttack;
					attackNum = i;
				}
			}
		}
		else
		{
			if ( bestAttack.damage > currentAttack.damage && targetHealth - bestAttack.damage <= 0 && targetHealth - currentAttack.damage <= 0 )
			{
				bestAttack = currentAttack;
				attackNum = i;
			}
			else if ( bestAttack.damage < currentAttack.damage )
			{
				bestAttack = currentAttack;
				attackNum = i;
			}
		}
	}
	return attackNum;
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
}

// Used to get a string of the attack for outputing the name of the attack
string CCharEntity::GetAttackElement()
{
	if ( m_CurrentAttack == -1 )
	{
		return "???";
	}
	SAttack attack = m_CharTemplate->GetAttack(m_CurrentAttack);
	if ( attack.element == Cut )
	{
		 return "Cut";
	}
	else if ( attack.element == Crush )
	{
		return "Crush";
	}
	else if ( attack.element == Stab )
	{
		return "Stab";
	}
	else if ( attack.element == Lightning )
	{
		return "Lightning";
	}
	else if ( attack.element == Fire )
	{
		return "Fire";
	}
	else if ( attack.element == Ice )
	{
		return "Ice";
	}
	else if ( attack.element == Arcane )
	{
		return "Arcane";
	}
	else
	{
		return "???";
	}
}
// Used to get a string for the output of whether or not the character is defending
string CCharEntity::GetDefenceInfo()
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

void CCharEntity::RandomAttack( SMessage msg )
{
	// Choosing Attack
	m_CurrentAttack = Random(0, m_CharTemplate->GetAttackNum() - 1);
	if ( m_CharTemplate->GetAttack(m_CurrentAttack).type == Magical )
	{
		if ( m_CurrentMagic - m_CharTemplate->GetAttack(m_CurrentAttack).cost >= 0 )
		{
			m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).cost;
		}
		else
		{
			do
			{
				m_CurrentAttack = Random(0, m_CharTemplate->GetAttackNum() - 1);
			} while ( m_CurrentMagic - m_CharTemplate->GetAttack(m_CurrentAttack).cost < 0 );
		}
	}

	// Choosing Target
	TEntityUID target;
	if( m_CharTemplate->GetType() == "enemy" )
	{
		target = RandomAlly();
	}
	else
	{
		target = RandomEnemy();
	}

	msg.type = Msg_Attacked;
	msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack);
	if(msg.attack.type == Physical)
	{
		msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f);
	}
	else
	{
		msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f);
	}
	msg.from = GetUID();
	if(effectOn)
	{
		attackEffect.StartAttack(Matrix().GetPosition(), target, msg);
	}
	else
	{
		Messenger.SendMessage( target, msg );
		msg.type = Msg_Act;
		msg.order++;
		if(msg.order >= NumTotal)
		{
			msg.order = 0;
		}
		Messenger.SendMessage(AttackOrder[msg.order],msg);
	}
	m_State = Wait;
}
void CCharEntity::ReacativeAttack( SMessage msg )
{
	if ( m_CurrentHealth < m_CharTemplate->GetMaxHealth() / 4 )
	{
		int chance = Random( 0, 10 );
		if ( chance < 7 )
		{
			int cost;
			do
			{
				chance = Random( 0, m_CharTemplate->GetDefenceNum()-1);
				cost = m_CharTemplate->GetDefence(chance).cost;
			} while ( m_CurrentMagic - cost < 0 );
			m_CurrentDefence = chance;
			m_Defend = true;
			m_CurrentMagic -= cost;
		}
	}

	if(!m_Defend && !UseItem(msg))
	{
		TEntityUID target;

		if(Template()->GetType() == "enemy")
		{
			target = LowestHealthAlly();
		}
		else
		{
			target = LowestHealthEnemy();
		}

		int targetHealth = EntityManager.GetCharEntity(target)->GetCurrentHealth();
		bool foeWeakness = false; // Used to deside which attack to use based on if the target has a weakness
		int weaknessAttack = 0;

		for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
		{
			if(EntityManager.GetCharEntity(target)->m_CharTemplate->GetWeakness() == m_CharTemplate->GetAttack(i).element)
			{
				weaknessAttack = i;
				foeWeakness = true;
			}
		}

		if(foeWeakness && m_CurrentMagic - m_CharTemplate->GetAttack(weaknessAttack).cost > 0)
		{
			m_CurrentAttack = weaknessAttack;
		}
		else
		{
			m_CurrentAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic);
		}
		m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).cost;

		msg.type = Msg_Attacked;
		msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack);
		if(msg.attack.type == Physical)
		{
			msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f);
		}
		else
		{
			msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f);
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
	if(!effectOn)
	{
		msg.type = Msg_Act;
		msg.order++;
		if(msg.order >= NumTotal)
		{
			msg.order = 0;
		}
		Messenger.SendMessage(AttackOrder[msg.order],msg);
	}
	m_State = Wait;
}
void CCharEntity::Planning( SMessage msg )
{
	if(!UseItem(msg))
	{
		std::string type = m_CharTemplate->GetType();
		if(m_CurrentHealth < m_CharTemplate->GetMaxHealth() / 4)
		{
			int chance = Random(0,10);
			if(chance < 7 && !m_DefendLast)
			{
				int averageStrength = 0;
				int averageIntelligence = 0;

				if(type == "enemy")
				{
					for(auto it = Allies.begin(); it != Allies.end(); it++)
					{
						averageStrength += EntityManager.GetCharEntity(*it)->m_CharTemplate->GetStrength();
					}
					averageStrength /= Allies.size();
					for(auto it = Allies.begin(); it != Allies.end(); it++)
					{
						averageIntelligence += EntityManager.GetCharEntity(*it)->m_CharTemplate->GetIntelligence();
					}
					averageIntelligence /= Allies.size();
				}
				else
				{
					for(auto it = Enemies.begin(); it != Enemies.end(); it++)
					{
						averageStrength += EntityManager.GetCharEntity(*it)->m_CharTemplate->GetStrength();
					}
					averageStrength /= Enemies.size();
					for(auto it = Enemies.begin(); it != Enemies.end(); it++)
					{
						averageIntelligence += EntityManager.GetCharEntity(*it)->m_CharTemplate->GetIntelligence();
					}
					averageIntelligence /= Enemies.size();
				}
				if(averageStrength > averageIntelligence)
				{
					do
					{
						m_CurrentDefence = Random(0,m_CharTemplate->GetDefenceNum()-1);
					} while(m_CurrentMagic - m_CharTemplate->GetDefence(m_CurrentDefence).cost < 0 && m_CharTemplate->GetDefence(m_CurrentDefence).type != Physical 
						    && m_CharTemplate->GetDefence(m_CurrentDefence).type != Both);
				}
				else
				{
					do
					{
						m_CurrentDefence = Random(0,m_CharTemplate->GetDefenceNum()-1);
					} while(m_CurrentMagic - m_CharTemplate->GetDefence(m_CurrentDefence).cost < 0 && m_CharTemplate->GetDefence(m_CurrentDefence).type != Magical 
						    && m_CharTemplate->GetDefence(m_CurrentDefence).type != Both);
				}
				m_CurrentMagic -= m_CharTemplate->GetDefence(m_CurrentDefence).cost;

				m_Defend = true;
				m_DefendLast = true;
			}
			else
			{
				m_DefendLast = false;
			}
		}
		if(!m_Defend)
		{
			TEntityUID target;

			if(type == "enemy")
			{
				bool weakness = false;
				for(auto it = Allies.begin(); it != Allies.end(); it++)
				{
					EElement targetWeakness = EntityManager.GetCharEntity(*it)->m_CharTemplate->GetWeakness();
					bool targetDead = EntityManager.GetCharEntity(*it)->isDead();
					for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
					{
						if(targetWeakness == m_CharTemplate->GetAttack(i).element && !targetDead)
						{
							weakness = true;
							target = *it;
						}
					}
				}
				if(!weakness )
				{
					target = LowestHealthAlly();
				}
			}
			else
			{
				bool weakness = false;
				for(auto it = Enemies.begin(); it != Enemies.end(); it++)
				{
					EElement targetWeakness = EntityManager.GetCharEntity(*it)->m_CharTemplate->GetWeakness();
					bool targetDead = EntityManager.GetCharEntity(*it)->isDead();
					for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
					{
						if(targetWeakness == m_CharTemplate->GetAttack(i).element && !targetDead)
						{
							weakness = true;
							target = *it;
						}
					}
				}
				if(!weakness)
				{
					target = LowestHealthEnemy();
				}
			}

			int targetHealth = EntityManager.GetCharEntity(target)->GetCurrentHealth();
			bool targetWeakness = false;
			int weaknessAttack = 0;

			for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
			{
				if(EntityManager.GetCharEntity(target)->m_CharTemplate->GetWeakness() == m_CharTemplate->GetAttack(i).element)
				{
					weaknessAttack = i;
					targetWeakness = true;
				}
			}
			int bestAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic);

			if(bestAttack != weaknessAttack)
			{
				SAttack weaknessA = m_CharTemplate->GetAttack(weaknessAttack);
				SAttack bestA = m_CharTemplate->GetAttack(bestAttack);

				float weaknessDamage = weaknessA.damage * 2;
				float bestDamage = bestA.damage;

				if(weaknessA.type == Physical)
				{
					weaknessDamage = weaknessDamage * m_CharTemplate->GetStrength() / 100.0f + 0.5f;
				}
				else
				{
					weaknessDamage = weaknessDamage * m_CharTemplate->GetIntelligence() / 100.0f + 0.5f;
				}

				if(bestA.type == Physical)
				{
					bestDamage = bestDamage * m_CharTemplate->GetStrength() / 100.0f + 0.5f;
				}
				else
				{
					bestDamage = bestDamage * m_CharTemplate->GetIntelligence() / 100.0f + 0.5f;
				}

				if((weaknessDamage > bestDamage) && targetWeakness)
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
			m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).cost;

			msg.type = Msg_Attacked;
			msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack);
			if(msg.attack.type == Physical)
			{
				msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetStrength()) / 100.0f) + 0.5f);
			}
			else
			{
				msg.attack.damage = (((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetIntelligence()) / 100.0f) + 0.5f);
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
	if(!effectOn)
	{
		msg.type = Msg_Act;
		msg.order++;
		if(msg.order >= NumTotal)
		{
			msg.order = 0;
		}
		Messenger.SendMessage(AttackOrder[msg.order],msg);
	}
	m_State = Wait;
}

//The cost of defence is decided when it is chosen so the check is not needed for the cost.
void CCharEntity::TakingDamage( SMessage msg )
{
	float defence = 1.0f;  //A modifier for the damage taken by the character.
	if ( m_Defend )
	{
		SDefence pick = m_CharTemplate->GetDefence(m_CurrentDefence);
		if ( pick.type == Regular )
		{
			if ( pick.attackRecivedType == msg.attack.type || pick.attackRecivedType == Both )
			{
				if ( pick.element == None || pick.element == msg.attack.element )
				{
						defence = pick.modifier;
				}
			}
		}
		else if ( pick.type == Reflect )
		{
				defence = 0.0f;
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.modifier;
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
		}
		else if ( pick.type == PainSplit )
		{
				defence = pick.modifier;
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.modifier;
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
		}

		m_Defend = false;
		m_CurrentDefence = -1;
	}
	if ( msg.attack.element == m_CharTemplate->GetWeakness() )
	{
		defence *= 2;
	}
	m_CurrentHealth -= static_cast<TInt32>(msg.attack.damage * defence);
}

bool CCharEntity::Update( TFloat32 updateTime )
{
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
			case Msg_HealthRestored:
				m_CurrentHealth += msg.itemEffect;
				if ( m_CurrentHealth > m_CharTemplate->GetMaxHealth() )
				{
					m_CurrentHealth = m_CharTemplate->GetMaxHealth();
				}
				break;
			case Msg_MagicRestored:
				m_CurrentMagic += msg.itemEffect;
				if(m_CurrentMagic > m_CharTemplate->GetMaxMagic())
				{
					m_CurrentMagic = m_CharTemplate->GetMaxMagic();
				}
				break;
			case Msg_Revive:
				if (m_State == Dead)
				{
					Matrix().SetY(0.0f);
					m_State = Wait;
					m_CurrentHealth = m_CharTemplate->GetMaxHealth() / 2;
				}
				break;
			case Msg_Poison:
				m_Poison = IsPoisoned;
				m_PoisonCount = 5;
				break;
		}
	}

	if ( m_CurrentHealth <= 0 && m_State != Dead )
	{
		Matrix().SetY(-200.0f);
		m_State = Dead;
	}

	if( m_Inventory.size() > 0 )
	{
		vector<int> none;
		for(int i = 0; i < m_Inventory.size(); i++)
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
			for(int i = 0; i < none.size(); i++)
			{
				m_Inventory.erase(m_Inventory.begin() + none[i]);
			}
		}
	}

	if (m_State == Act)
	{
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
			int AIUsed;
			if(templateAIOn)
			{
				AIUsed = m_CharTemplate->GetAI();
			}
			else
			{
				AIUsed = generalAI;
			}

			if ( AIUsed == 1 )
			{
				RandomAttack( msg );
			}
			if ( AIUsed == 2 )
			{
				ReacativeAttack( msg );
			}
			if ( AIUsed == 3 )
			{
				Planning( msg );
			}
		}
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

	if (m_State == Dead)
	{
		msg.type = Msg_Act;
		msg.order++;
		if (msg.order >= NumTotal)
		{
			msg.order = 0;
		}
		Messenger.SendMessage(AttackOrder[msg.order], msg);
	}

	return true;
}

void CCharEntity::AddItemToInvantory( SItem newItem )
{
	bool haveOne = false;
	int haveOnePos = 0;
	for ( int i = 0; i < m_Inventory.size(); i++ )
	{
		if ( m_Inventory[i].item.name == newItem.name )
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

bool CCharEntity::UseItem( SMessage msg )
{
	if ( m_Inventory.size() == 0 )
	{
		return false;
	}
	else
	{
		for( int i = 0; i < m_Inventory.size(); i++ )
		{
			if ( m_Inventory[i].item.effect == restoreHealth )
			{
				TEntityUID current;
				CCharEntity* hurt;
				if ( m_CharTemplate->GetType() == "Ally" )
				{
					current = LowestHealthAlly();
					hurt = EntityManager.GetCharEntity( current );
				}
				else
				{
					current = LowestHealthEnemy();
					hurt = EntityManager.GetCharEntity( current );
				}

				if ( hurt->GetCurrentHealth() < (hurt->m_CharTemplate->GetMaxHealth() / 10) )
				{
					msg.type = Msg_HealthRestored;
					msg.from = GetUID();
					msg.itemEffect = m_Inventory[i].item.value;
					Messenger.SendMessage(current, msg);
					m_Inventory[i].Quantity--;
					return true;
				}

			}
			else if ( m_Inventory[i].item.effect == restoreMana )
			{
				TEntityUID current;
				CCharEntity* lowMagic;
				if ( m_CharTemplate->GetType() == "Ally" )
				{
					current = LowestMagicAlly();
					lowMagic = EntityManager.GetCharEntity( current );
				}
				else
				{
					current = LowestMagicEnemy();
					lowMagic = EntityManager.GetCharEntity( current );
				}

				if ( lowMagic->GetCurrentMagic() < (lowMagic->m_CharTemplate->GetMaxMagic() * 0.1) )
				{
					msg.type = Msg_MagicRestored;
					msg.from = GetUID();
					msg.itemEffect = m_Inventory[i].item.value;
					Messenger.SendMessage(current, msg);
					m_Inventory[i].Quantity--;
					return true;
				}
			}
			else if ( m_Inventory[i].item.effect == revive )
			{
				TEntityUID target;
				if ( m_CharTemplate->GetType() == "Enemy" )
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
					msg.itemEffect = m_Inventory[i].item.value;
					Messenger.SendMessage(target, msg);
					m_Inventory[i].Quantity--;
					return true;
				}
			}
			/*else if ( m_Inventory[i].item.effect == poison )
			{
				if ( m_Poison != PoisonedWeapon )
				{
					m_Poison = PoisonedWeapon;
					m_Inventory[i].Quantity--;
					return true;
				}
			}*/
		}
		return false;
	}
	return false;
}

TInt32 CCharEntity::GetNumInInvantory()
{
	int total = 0;
	for ( int i = 0; i < m_Inventory.size(); i++ )
	{
		total += m_Inventory[i].Quantity;
	}
	return total;
}

} //namespace gen