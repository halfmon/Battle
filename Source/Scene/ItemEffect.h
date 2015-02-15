#pragma once

#include <string>

#include "Defines.h"
#include "CVector3.h"
#include "Entity.h"
#include "Messenger.h"

namespace gen
{

const int ITEM_SPEED = 7;

enum EItemState { ActiveI,InactiveI };

class CItemEffect
{
private:
	CVector3 m_TargetPos;

	TEntityUID m_Target;
	vector<TEntityUID> m_Effect;
	int m_CurrentEffect;

	EItemState m_State;
	SMessage m_MSG;

	float m_LifeTime;

public:
	CItemEffect(void);
	CItemEffect(vector<std::string>,std::string);

	void Update(TFloat32 updateTime);

	void StartEffect(CVector3 attackerPos,TEntityUID target,SMessage msg);

	EItemState getState()
	{
		return m_State;
	}

	void Reset();
};

}