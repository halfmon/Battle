#include "ItemEffect.h"
#include "EntityManager.h"
#include "Externals.h"

namespace gen
{

CItemEffect::CItemEffect()
{
}

CItemEffect::CItemEffect(vector<std::string> tempName,std::string name)
{
	for(auto it = tempName.begin(); it != tempName.end(); it++)
	{
		m_Effect.push_back(EntityManager.CreateEntity((*it),name,CVector3(0.0f,-10.0f,0.0f),CVector3(0.0f,0.0f,0.0f),CVector3(0.05f,0.05f,0.05f)));
	}

	m_TargetPos = CVector3(0.0f,0.0f,0.0f);
	m_Target = NULL;

	m_State = InactiveI;
}

void CItemEffect::Update(TFloat32 updateTime)
{
	if(m_State == ActiveI)
	{
		m_LifeTime -= updateTime;
		CEntity* item = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);
		item->Matrix().MoveY(ITEM_SPEED * updateTime);

		if(Distance(m_TargetPos, item->Position()) < 0.2f || m_LifeTime <= 0)
		{
			if(m_MSG.type != Msg_AddPoison)
			{
				Messenger.SendMessage(m_Target,m_MSG);
			}
			item->Matrix().SetPosition(CVector3(0.0f,-10.0f,0.0f));

			m_TargetPos = CVector3(0.0f,0.0f,0.0f);
			m_Target = NULL;
			m_State = InactiveI;
		}
	}
}

void CItemEffect::StartEffect(CVector3 attackerPos,TEntityUID target,SMessage msg)
{
	switch(msg.type)
	{
	case Msg_HealthRestored:
		m_CurrentEffect = 0;
		break;
	case Msg_MagicRestored:
		m_CurrentEffect = 1;
		break;
	case Msg_AddPoison:
		m_CurrentEffect = 2;
		break;
	case Msg_Revive:
		m_CurrentEffect = 3;
		break;
	}
	m_LifeTime = 5.0f;

	//m_CurrentEffect = msg.item.effect;

	CEntity* effect = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);

	m_State = ActiveI;

	m_TargetPos = attackerPos;
	m_TargetPos.y += 3;

	m_Target = target;
	attackerPos.y += 1;
	effect->Matrix().SetPosition(attackerPos);
//	effect->Matrix().FaceTarget(m_TargetPos);
	m_MSG = msg;
}

void CItemEffect::Reset()
{
	m_TargetPos = CVector3(0.0f,0.0f,0.0f);
	m_Target = NULL;
	m_State = InactiveI;
	EntityManager.GetEntity(m_Effect[m_CurrentEffect])->Matrix().SetY(-10.0f);
}

}