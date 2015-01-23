#pragma once

#include <string>

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"
#include "Messenger.h"

namespace gen
{
const int ATTACK_SPEED = 10;

enum eAttackState { Active, Inactive };

class CAttackEffect
{
private:
	CVector3 m_TargetPos;

	TEntityUID m_Target;
	vector<TEntityUID> m_Effect;

	eAttackState m_State;
	SMessage m_MSG;

	int m_CurrentEffect;

public:
	CAttackEffect(void);
	CAttackEffect( vector<std::string>, std::string );
	~CAttackEffect(void);

	void Update(TFloat32 updateTime);

	void StartAttack( CVector3 attackerPos, TEntityUID target, SMessage msg );

	void Reset();

	TEntityUID getTarget()
	{
		return m_Target;
	}
	CVector3 getTargetPos()
	{
		return m_TargetPos;
	}
	eAttackState getState()
	{
		return m_State;
	}
	CVector3 getPos();
};

}

