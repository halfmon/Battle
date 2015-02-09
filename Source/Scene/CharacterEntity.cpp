/************************************************
	CharacterEntity.cpp

	Character entity template and entity classes
*************************************************/

#include "CharacterEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
//#include "AttackEffect.h"
//#include "ItemEffect.h"
#include "Externals.h"

namespace gen
{

/* Picks the best attack based only on the damage value of the attack. If two attacks could kill the target based on the attacks damage it will choose the 
attack that cost the least if it is magical. */
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

// Used to get a string of the attack for outputing the name of the attack
string CCharEntity::GetAttackElement()
{
	if ( m_CurrentAttack == -1 )
	{
		return "???";
	}
	CAttack attack = m_CharTemplate->GetAttack(m_CurrentAttack);
	if ( attack.GetElement() == Cut )
	{
		 return "Cut";
	}
	else if ( attack.GetElement() == Crush )
	{
		return "Crush";
	}
	else if ( attack.GetElement() == Stab )
	{
		return "Stab";
	}
	else if ( attack.GetElement() == Lightning )
	{
		return "Lightning";
	}
	else if ( attack.GetElement() == Fire )
	{
		return "Fire";
	}
	else if ( attack.GetElement() == Ice )
	{
		return "Ice";
	}
	else if ( attack.GetElement() == Arcane )
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
void CCharEntity::WeaknessAttack( SMessage msg )
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
				cost = m_CharTemplate->GetDefence(chance).GetCost();
			} while ( m_CurrentMagic - cost < 0 );
			m_CurrentDefence = chance;
			m_Defend = true;
			m_CurrentMagic -= cost;
		}
	}
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

		if(Template()->GetType() == "enemy")
		{
			bool weakness = false;
			for(auto it = Allies.begin(); it != Allies.end(); it++)
			{
				EAttackElement targetWeakness = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetWeakness().element;
				bool targetDead = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead();
				for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
				{
					if(targetWeakness == m_CharTemplate->GetAttack(i).GetElement() && !targetDead)
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
				EAttackElement targetWeakness = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->m_CharTemplate->GetWeakness().element;
				bool targetDead = static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead();
				for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
				{
					if(targetWeakness == m_CharTemplate->GetAttack(i).GetElement() && !targetDead)
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
		int targetHealth = static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth();
		bool targetWeakness = false;
		int weaknessAttack = 0;
		for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
		{
			if(static_cast<CCharEntity*>(EntityManager.GetEntity(target))->m_CharTemplate->GetWeakness().element == m_CharTemplate->GetAttack(i).GetElement())
			{
				weaknessAttack = i;
				targetWeakness = true;
			}
		}
		int bestAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic);

		if(bestAttack != weaknessAttack)
		{
			CAttack weaknessA = m_CharTemplate->GetAttack(weaknessAttack);
			CAttack bestA = m_CharTemplate->GetAttack(bestAttack);

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
		m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).GetCost();

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
void CCharEntity::StrongestAttack( SMessage msg )
{
	//Check to see
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
				if(averageStrength > averageIntelligence)
				{
					do
					{
						m_CurrentDefence = Random(0,m_CharTemplate->GetDefenceNum()-1);
					} while (m_CurrentMagic - m_CharTemplate->GetDefence(m_CurrentDefence).GetCost() < 0 && m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Physical
						&& m_CharTemplate->GetDefence(m_CurrentDefence).GetAttackRecivedType() != Both);
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
		if(!m_Defend)
		{
			TEntityUID target;

			if(type == "enemy")
			{
				target = LowestHealthAlly();
			}
			else
			{
				target = LowestHealthEnemy();
			}

			int targetHealth = static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth();
			bool foeWeakness = false; // Used to deside which attack to use based on if the target has a weakness
			int weaknessAttack = 0;

			for(int i = 0; i < m_CharTemplate->GetAttackNum(); i++)
			{
				if(static_cast<CCharEntity*>(EntityManager.GetEntity(target))->m_CharTemplate->GetWeakness().element == m_CharTemplate->GetAttack(i).GetElement())
				{
					weaknessAttack = i;
					foeWeakness = true;
				}
			}

			if(foeWeakness && m_CurrentMagic - m_CharTemplate->GetAttack(weaknessAttack).GetCost() > 0)
			{
				m_CurrentAttack = weaknessAttack;
			}
			else
			{
				m_CurrentAttack = PickBestAttack(m_CharTemplate,targetHealth,m_CurrentMagic);
			}
			m_CurrentMagic -= m_CharTemplate->GetAttack(m_CurrentAttack).GetCost();

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
	if ( m_Defend && msg.attack.type != Recoil )
	{
		CDefence pick = m_CharTemplate->GetDefence(m_CurrentDefence);
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
		else if (pick.GetType() == Reflect)
		{
				defence = 0.0f;
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.GetModifier();
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
		}
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
	if ( msg.attack.element == m_CharTemplate->GetWeakness().element )
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
				m_CurrentHealth += msg.item.value;
				if ( m_CurrentHealth > m_CharTemplate->GetMaxHealth() )
				{
					m_CurrentHealth = m_CharTemplate->GetMaxHealth();
				}
				break;
			case Msg_MagicRestored:
				m_CurrentMagic += msg.item.value;
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
		Matrix().SetY(-10.0f);
		m_State = Dead;
	}

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
				WeaknessAttack( msg );
			}
			if ( AIUsed == 3 )
			{
				StrongestAttack( msg );
			}

			numTurns++;

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

	}

	if(m_State == Dead)
	{
		if(effectOn)
		{
			if(attackEffect.getState() == Inactive)
			{
				msg.type = Msg_Act;
				msg.order++;
				if(msg.order >= NumTotal)
				{
					msg.order = 0;
				}
				Messenger.SendMessage(AttackOrder[msg.order],msg);
			}
		}
		else
		{
			msg.type = Msg_Act;
			msg.order++;
			if(msg.order >= NumTotal)
			{
				msg.order = 0;
			}
			Messenger.SendMessage(AttackOrder[msg.order],msg);
		}
	}

	return true;
}

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

bool CCharEntity::UseItem( SMessage msg )
{
	if ( m_Inventory.size() == 0 )
	{
		return false;
	}
	else
	{
		for(int i = 0; i < static_cast<int>(m_Inventory.size()); i++)
		{
			if ( m_Inventory[i].item.GetEffect() == restoreHealth )
			{
				TEntityUID current;
				CCharEntity* hurt;
				if ( m_CharTemplate->GetType() == "Ally" )
				{
					current = LowestHealthAlly();
					hurt = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}
				else
				{
					current = LowestHealthEnemy();
					hurt = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}

				if ( hurt->GetCurrentHealth() < (hurt->m_CharTemplate->GetMaxHealth() / 10) )
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
			else if ( m_Inventory[i].item.GetEffect() == restoreMana )
			{
				TEntityUID current;
				CCharEntity* lowMagic;
				if ( m_CharTemplate->GetType() == "Ally" )
				{
					current = LowestMagicAlly();
					lowMagic = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}
				else
				{
					current = LowestMagicEnemy();
					lowMagic = static_cast<CCharEntity*>(EntityManager.GetEntity( current ));
				}

				if ( lowMagic->GetCurrentMagic() < (lowMagic->m_CharTemplate->GetMaxMagic() * 0.1) )
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
			else if ( m_Inventory[i].item.GetEffect() == revive )
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

TInt32 CCharEntity::GetNumInInvantory()
{
	int total = 0;
	for ( int i = 0; i < static_cast<int>(m_Inventory.size()); i++ )
	{
		total += m_Inventory[i].Quantity;
	}
	return total;
}

} //namespace gen