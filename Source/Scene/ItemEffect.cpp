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
		CEntity* item = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);
		item->Matrix().MoveLocalZ(ITEM_SPEED * updateTime);

		TFloat32 X = fabs(item->Position().x - m_TargetPos.x);
		TFloat32 Z = fabs(item->Position().z - m_TargetPos.z);

		if(X < 0.2f && Z < 0.2f)
		{
			if(m_MSG.type != Msg_AddPoison)
			{
				Messenger.SendMessage(m_Target,m_MSG);
			}
			m_MSG.type = Msg_Act;
			m_MSG.order++;
			if(m_MSG.order >= NumTotal)
			{
				m_MSG.order = 0;
			}
			Messenger.SendMessage(AttackOrder[m_MSG.order],m_MSG);
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

	CEntity* effect = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);

	m_State = ActiveI;

	if(target == msg.from)
	{
		m_TargetPos = attackerPos;
		m_TargetPos.y += 5;
	}
	else
	{
		m_TargetPos = EntityManager.GetCharEntity(target)->Position();
		m_TargetPos.y += 1;
	}

	m_Target = target;
	attackerPos.y += 1;
	effect->Matrix().SetPosition(attackerPos);
	effect->Matrix().FaceTarget(m_TargetPos);
	m_MSG;
}

void CItemEffect::Reset()
{
	m_TargetPos = CVector3(0.0f,0.0f,0.0f);
	m_Target = NULL;
	m_State = InactiveI;
	EntityManager.GetEntity(m_Effect[m_CurrentEffect])->Matrix().SetY(-10.0f);
}

}