/*******************************************
	
	Light.h

	Light class declarations

********************************************/

#pragma once

#include "Defines.h"
#include "CVector3.h"
#include "Colour.h"
#include "Input.h"

namespace gen
{

class CLight
{
public:
	// Constructor / Destructor
	CLight( const CVector3& pos, const SColourRGBA& col, TFloat32 atten = 100.0f );


	// Getters
	CVector3 GetPosition()
	{
		return m_Position;
	}
	SColourRGBA GetColour()
	{
		return m_Colour;
	}
	TFloat32 GetAttenuation()
	{
		return m_Atten;
	}

	// Setters
	void SetPosition( const CVector3& pos )
	{
		m_Position = pos;
	}
	void SetColour( const SColourRGBA& col )
	{
		m_Colour = col;
	}
	void SetAttenuation( float atten )
	{
		m_Atten = atten;
	}


	// Control the light with keys
	void Control( EKeyCode moveForward, EKeyCode moveBackward,
	              EKeyCode moveLeft, EKeyCode moveRight,
	              EKeyCode moveUp, EKeyCode moveDown,
				  TFloat32 MoveSpeed );


private:
	// Light settings
	CVector3    m_Position;
	SColourRGBA m_Colour;
	TFloat32    m_Atten;
};


} // namespace gen