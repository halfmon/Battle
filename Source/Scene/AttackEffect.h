#pragma once

#include <string>

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"
#include "Messenger.h"

namespace gen
{
	const int ATTACK_SPEED = 10;

	enum eState { Active, Inactive };

class CAttackEffect
{
private:
	CVector3 m_TargetPos;

	TEntityUID m_Target;
	TEntityUID m_Effect;

	eState m_State;
	SMessage m_MSG;

public:
	CAttackEffect(void);
	CAttackEffect( std::string, std::string );
	~CAttackEffect(void);

	void Update(TFloat32 updateTime);

	void StartAttack( CVector3 targetPos, CVector3 attackerPos, TEntityUID target, SMessage msg );

	void Reset();
};

}

