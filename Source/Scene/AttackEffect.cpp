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
	for(auto it = tempName.begin(); it != tempName.end(); it++)
	{
		m_Effect.push_back(EntityManager.CreateEntity((*it), name, CVector3(0.0f,-10.0f,0.0f),CVector3(0.0f,0.0f,0.0f),CVector3(0.05f,0.05f,0.05f)));
	}
	
	m_TargetPos = CVector3(0.0f, 0.0f, 0.0f);
	m_Target = NULL;
	m_CurrentEffect = 0;

	m_State = Inactive;
}


CAttackEffect::~CAttackEffect(void)
{
}

void CAttackEffect::Update( TFloat32 updateTime )
{
	if ( m_State == Active )
	{
		m_LifeTime -= updateTime;
		CEntity* attack = EntityManager.GetEntity(m_Effect[m_CurrentEffect]);
		attack->Matrix().MoveLocalZ( ATTACK_SPEED * updateTime );

		if(Distance(m_TargetPos, attack->Position()) < 0.2f || m_LifeTime <= 0)
		{
			Messenger.SendMessage( m_Target, m_MSG );
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

	m_LifeTime = 5.0f;

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

}
