#include "Items.h"
#include "EntityManager.h"
#include "Externals.h"

namespace gen
{

CItem::CItem(std::string name,EItemEffect effect,TInt32 value)
{
	m_Name = name;
	m_Effect = effect;
	m_Value = value;
}

SItem CItem::Use(TEntityUID user)
{
	SItem item;
	item.effect = m_Effect;
	item.name = m_Name;
	item.value = m_Value;
	return item;
}

}