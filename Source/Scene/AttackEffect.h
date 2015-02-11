#pragma once

#include <string>

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"
#include "Messenger.h"

namespace gen
{
const int ATTACK_SPEED = 10;

enum EAttackState { Active, Inactive };

class CAttackEffect
{
private:
	CVector3 m_TargetPos;

	TEntityUID m_Target;
	vector<TEntityUID> m_Effect;

	EAttackState m_State;
	SMessage m_MSG;

	int m_CurrentEffect;

	float m_LifeTime;

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
	EAttackState getState()
	{
		return m_State;
	}

};

}

