#include "AttackEffect.h"
#include "EntityManager.h"

namespace gen
{
extern CEntityManager EntityManager;
extern CMessenger Messenger;

extern TInt32 NumTotal;
extern vector<TEntityUID> AttackOrder;

CAttackEffect::CAttackEffect(void)
{
}

CAttackEffect::CAttackEffect(std::string tempName, std::string name)
{
	m_Effect = EntityManager.CreateEntity( tempName, name, CVector3( 0, -100, 0 ), CVector3( 0.0f, 0.0f, 0.0f ), CVector3( 0.05f, 0.05f, 0.05f) );

	m_TargetPos = CVector3( 0, 0, 0 );
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
		CEntity * attack = EntityManager.GetEntity(m_Effect);
		attack->Matrix().FaceTarget( m_TargetPos );
		attack->Matrix().MoveLocalZ( ATTACK_SPEED * updateTime );

		if( attack->Matrix().GetX() - m_TargetPos.x < 0.2f && attack->Matrix().GetZ() - m_TargetPos.z < 0.2f )
		{
			Messenger.SendMessage( m_Target, m_MSG );
			m_MSG.type = Msg_Act;
			m_MSG.order++;
			if(m_MSG.order >= NumTotal)
			{
				m_MSG.order = 0;
			} 
			Messenger.SendMessage(AttackOrder[m_MSG.order],m_MSG);
			attack->Matrix().SetPosition( CVector3(0.0f, -100.0f, 0.0f) );
			m_State = Inactive;
		}
	}
}

void CAttackEffect::StartAttack( CVector3 targetPos, CVector3 attackerPos, TEntityUID target, SMessage msg )
{
	m_TargetPos = targetPos;
	m_Target = target;
	attackerPos.y += 1;
	EntityManager.GetEntity(m_Effect)->Matrix().SetPosition( attackerPos );
	m_MSG = msg;
	m_State = Active;

	m_TargetPos.y += 1;

	switch( msg.attack.element )
	{
	case Cut:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Cut.png" );
		break;
	case Crush:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Crush.png" );
		break;
	case Stab:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Stab.png" );
		break;
	case Lightning:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Lightning.png" );
		break;
	case Fire:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Fire.png" );
		break;
	case Ice:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Ice.png" );
		break;
	case Arcane:
		EntityManager.GetEntity(m_Effect)->Template()->Mesh()->ChangeTexture( "Arcane.png" );
		break;
	}
}

void CAttackEffect::Reset()
{
	m_State = Inactive;
	EntityManager.GetEntity( m_Effect )->Matrix().SetY( -100 );
}

}
