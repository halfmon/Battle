#include "Attacks.h"
#include "EntityManager.h"
#include "Externals.h"

namespace gen
{

CAttack::CAttack(std::string name,EAttackType type,EAttackElement element,TInt32 cost,TFloat32 damage,vector<SWeakness> addWeakness, TFloat32 recoil)
{
	m_Name = name;
	m_Type = type;
	m_Element = element;
	m_Cost = cost;
	m_Damage = damage;
	m_Recoil = recoil;

	for(auto it = addWeakness.begin(); it != addWeakness.end(); it++)
	{
		m_AddWeakness.push_back(*it);
	}
}

SAttack CAttack::Attack(TEntityUID attacker)
{
	for(auto it = m_AddWeakness.begin(); it != m_AddWeakness.end(); it++)
	{
		if((*it).element != None)
		{
			EntityManager.GetCharEntity(attacker)->AddWeakness(*it);
		}
	}
	if(m_Recoil > 0.0f)
	{
		SMessage msg;
		SAttack recoil;
		recoil.cost = 0;
		recoil.damage = m_Damage * m_Recoil;
		recoil.element = Recoil;
		recoil.type = Both;
		msg.type = Msg_Attacked;
		msg.attack = recoil;
		Messenger.SendMessage(attacker,msg);
	}

	SAttack attack;
	attack.cost = m_Cost;
	attack.damage = m_Damage;
	attack.element = m_Element;
	attack.type = m_Type;

	return attack;
}

int CAttack::WeaknesHasEffect( vector<EAttackElement> attackElements )
{
	int attacks = 0;
	for( auto it = m_AddWeakness.begin(); it != m_AddWeakness.end(); it++ )
	{
		if( (*it).element = None )
		{
			return 0;
		}
		for( auto attackit = attackElements.begin(); attackit != attackElements.end(); it++ )
		{
			if( (*it).element == (*attackit) )
			{
				attacks++;
			}
		}
	}
	return attacks;
}

int CAttack::RecoilCalculation( int health )
{
	return (health - static_cast<int>(m_Damage * m_Recoil));
}

}