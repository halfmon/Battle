/************************************************
	CharacterEntity.cpp

	Character entity template and entity classes
*************************************************/

#include "CharacterEntity.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "AttackEffect.h"

namespace gen
{

extern CEntityManager EntityManager;
extern CMessenger Messenger;
extern CAttackEffect attackEffect;

extern TEntityUID RandomEnemy();
extern TEntityUID RandomAlly();
extern bool EnemyAlive();
extern bool AllyAlive();

extern TUInt32 ViewportWidth;
extern TUInt32 ViewportHeight;
extern CCamera* MainCamera;

extern vector<TEntityUID> AttackOrder;

extern int AIUsed;
extern TInt32 NumTotal;
extern bool effectOn;

extern TEntityUID LowestHealthAlly();
extern TEntityUID LowestHealthEnemy();

extern TEntityUID LowestMagicAlly();
extern TEntityUID LowestMagicEnemy();

extern TEntityUID DeadEnemy();
extern TEntityUID DeadAlly();

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
}

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
	m_CurrentAttack = Random(0, GetTemplate()->GetAttackNum() - 1);
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
				m_CurrentAttack = Random(0, GetTemplate()->GetAttackNum() - 1);
			} while ( m_CurrentMagic - m_CharTemplate->GetAttack(m_CurrentAttack).cost < 0 );
		}
	}

	TEntityUID target;
	if( Template()->GetType() == "enemy" )
	{
		target = RandomAlly();
	}
	else
	{
		target = RandomEnemy();
	}
	msg.type = Msg_Attacked;
	if ( m_CharTemplate->GetAttack(m_CurrentAttack).type == Physical )
	{
		msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack);
		msg.attack.damage = static_cast<TInt32>(((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetStrength()) / 100) + 0.5f);
	}
	else
	{
		msg.attack = m_CharTemplate->GetAttack(m_CurrentAttack);
		msg.attack.damage = static_cast<TInt32>(((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetIntelligence()) / 100) + 0.5f);
	}
	msg.from = GetUID();
	if(effectOn)
	{
		attackEffect.StartAttack(EntityManager.GetCharEntity(target)->Matrix().GetPosition(),Matrix().GetPosition(),target,msg);
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
				cost = GetTemplate()->GetDefence(chance).cost;
			} while ( m_CurrentMagic - cost < 0 );
			m_CurrentDefence = chance;
			m_Defend = true;
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
		if(m_CharTemplate->GetAttack(m_CurrentAttack).type == Physical)
		{
			msg.attack.damage = static_cast<TInt32>(((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetStrength()) / 100) + 0.5f);
		}
		else
		{
			msg.attack.damage = static_cast<TInt32>(((m_CharTemplate->GetAttack(m_CurrentAttack).damage * m_CharTemplate->GetIntelligence()) / 100) + 0.5f);
		}
		msg.from = GetUID();
		if(effectOn)
		{
			attackEffect.StartAttack(EntityManager.GetCharEntity(target)->Matrix().GetPosition(),Matrix().GetPosition(),target,msg);
		}
		else
		{
			Messenger.SendMessage(target,msg);
		}
		m_State = Wait;
		if(m_Poison == PoisonedWeapon)
		{
			m_Poison == None;
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
}
void CCharEntity::Planning( SMessage msg )
{
	if(UseItem(msg))
	{
		msg.type = Msg_Act;
		msg.order++;
		if(msg.order >= NumTotal)
		{
			msg.order = 0;
		}
		Messenger.SendMessage(AttackOrder[msg.order],msg);
		m_State = Wait;
	}
	else
	{
		if(m_CurrentHealth < m_CharTemplate->GetMaxHealth() / 4)
		{
			int chance = Random(0,10);
			if(chance < 7)
			{
				int cost;
				do
				{
					chance = Random(0,m_CharTemplate->GetDefenceNum()-1);
					cost = GetTemplate()->GetDefence(chance).cost;
				} while(m_CurrentMagic - cost < 0);
				m_CurrentDefence = chance;
				m_Defend = true;
			}
		}
		ReacativeAttack(msg);
	}
}

void CCharEntity::TakingDamage( SMessage msg )
{
	float defence = 1.0f;
	if ( m_Defend )
	{
		SDefence pick = GetTemplate()->GetDefence(m_CurrentDefence);
		if ( pick.type == Regular )
		{
			if ( pick.attackRecivedType == msg.attack.type )
			{
				if ( pick.element == None || pick.element == msg.attack.element )
				{
					defence = pick.modifier;
					m_CurrentMagic -= pick.cost;
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
				m_CurrentMagic -= pick.cost;
		}
		else if ( pick.type == PainSplit )
		{
				defence = pick.modifier;
				TEntityUID from = msg.from;
				msg.type = Msg_Attacked;
				msg.attack.damage *= pick.modifier;
				msg.from = GetUID();
				Messenger.SendMessage( from, msg );
				m_CurrentMagic -= pick.cost;
		}

		m_Defend = false;
		m_CurrentDefence = -1;
	}
	if ( msg.attack.element == GetTemplate()->GetWeakness() )
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
				if ( m_CurrentHealth > GetTemplate()->GetMaxHealth() )
				{
					m_CurrentHealth = GetTemplate()->GetMaxHealth();
				}
				break;
			case Msg_MagicRestored:
				m_CurrentMagic += msg.itemEffect;
				if(m_CurrentMagic > GetTemplate()->GetMaxMagic())
				{
					m_CurrentMagic = GetTemplate()->GetMaxMagic();
				}
				break;
			case Msg_Revive:
				if (m_State == Dead)
				{
					Matrix().SetY(0.0f);
					m_State = Wait;
					m_CurrentHealth = GetTemplate()->GetMaxHealth() / 2;
					AttackOrder.push_back(GetUID());
					NumTotal++;
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

	vector<int> none;
	for ( int i = 0; i < m_Inventory.size(); i++ )
	{
		if( m_Inventory[i].Quantity <= 0 )
		{
			none.push_back( i );
		}
	}

	for ( int i = 0; i < none.size(); i++ )
	{
		m_Inventory.erase(m_Inventory.begin() + none[i] );
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

	if(m_Poison == IsPoisoned)
	{
		m_PoisonCount--;
		m_CurrentHealth *= 0.9f;
		if(m_PoisonCount < 1)
		{
			m_Poison = None;
		}
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
				if ( GetTemplate()->GetType() == "Ally" )
				{
					current = LowestHealthAlly();
					hurt = EntityManager.GetCharEntity( current );
				}
				else
				{
					current = LowestHealthEnemy();
					hurt = EntityManager.GetCharEntity( current );
				}

				if ( hurt->GetCurrentHealth() < (hurt->GetTemplate()->GetMaxHealth() / 10) )
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
				if ( GetTemplate()->GetType() == "Ally" )
				{
					current = LowestMagicAlly();
					lowMagic = EntityManager.GetCharEntity( current );
				}
				else
				{
					current = LowestMagicEnemy();
					lowMagic = EntityManager.GetCharEntity( current );
				}

				if ( lowMagic->GetCurrentMagic() < (lowMagic->GetTemplate()->GetMaxMagic() * 0.1) )
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
				if ( GetTemplate()->GetType() == "Enemy" )
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
			else if ( m_Inventory[i].item.effect == poison )
			{
				if ( m_Poison != PoisonedWeapon )
				{
					m_Poison = PoisonedWeapon;
					m_Inventory[i].Quantity--;
					return true;
				}
			}
		}
		return false;
	}
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