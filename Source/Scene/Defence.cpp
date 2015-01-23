#include "Defence.h"
#include "EntityManager.h"
#include "Externals.h"

namespace gen
{

CDefence::CDefence(EDefenceType type,EAttackType attackRecivedType,EAttackElement element,TFloat32 modifier,TInt32 cost)
{
	m_Type = type;
	m_AttackRecived = attackRecivedType;
	m_Element = element;
	m_Modifier = modifier;
	m_Cost = cost;
}

SDefence CDefence::Defend(TEntityUID defender)
{
	SDefence defence;
	defence.attackRecivedType = m_AttackRecived;
	defence.cost = m_Cost;
	defence.element = m_Element;
	defence.modifier = m_Modifier;
	defence.type = m_Type;
	return defence;
}

}