#include "AttackEffect.h"
#include "EntityManager.h"
#include "Externals.h"

namespace gen
{

CAttackEffect::CAttackEffect(void)
{
}

CAttackEffect::CAttackEffect(vector<std::string> tempName, std::string name)
{
	int count = 0;
	for(auto it = tempName.begin(); it != tempName.end(); it++)
	{
		m_Effect.push_back(EntityManager.CreateEntity((*it), name, CVector3(0.0f,-10.0f,0.0f),CVector3(0.0f,0.0f,0.0f),CVector3(0.05f,0.05f,0.05f)));
	}
	
	for(int i = 0; i < tempName.size(); i++)
	{
		m_TargetPos = CVector3(0.0f + (i*10),0.0f,0.0f);
	}
	m_Target = NULL;

	m_State = Inactive;
}


CAttackEffect::~CAttackEffect(void)
{
}

void CAttackEffect::Update( TFloat32 updateTime )
{
	if ( m_State == Active )
	{
		CEntity* attack = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);
		//attack->Matrix().FaceTarget( m_TargetPos );
		attack->Matrix().MoveLocalZ( ATTACK_SPEED * updateTime );

		TFloat32 X = fabs(attack->Position().x - m_TargetPos.x);
		TFloat32 Z = fabs(attack->Position().z - m_TargetPos.z);

		if( X < 0.2f && Z < 0.2f )
		{
			Messenger.SendMessage( m_Target, m_MSG );
			m_MSG.type = Msg_Act;
			m_MSG.order++;
			if(m_MSG.order >= NumTotal)
			{
				m_MSG.order = 0;
			} 
			Messenger.SendMessage(AttackOrder[m_MSG.order],m_MSG);
			attack->Matrix().SetPosition( CVector3(0.0f, -10.0f, 0.0f) );

			m_TargetPos = CVector3(0.0f,0.0f,0.0f);
			m_Target = NULL;
			m_State = Inactive;
		}
	}
}

void CAttackEffect::StartAttack( CVector3 attackerPos, TEntityUID target, SMessage msg )
{
	m_CurrentEffect = msg.attack.element;

	CEntity* effect = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);

	m_State = Active;

	m_TargetPos = EntityManager.GetCharEntity(target)->Position();
	m_TargetPos.y += 1;
	m_Target = target;
	attackerPos.y += 1;
	effect->Matrix().SetPosition( attackerPos );
	effect->Matrix().FaceTarget( m_TargetPos );
	m_MSG = msg;
}

void CAttackEffect::Reset()
{
	m_TargetPos = CVector3(0.0f,0.0f,0.0f);
	m_Target = NULL;
	m_State = Inactive;
	EntityManager.GetEntity( m_Effect[m_CurrentEffect] )->Matrix().SetY( -10.0f );
}

CVector3 CAttackEffect::getPos()
{
	return EntityManager.GetEntity( m_Effect[m_CurrentEffect] )->Position();
}

}
