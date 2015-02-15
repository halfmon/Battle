/*******************************************
	CarTag.cpp

	Shell scene and game functions
********************************************/

#include <sstream>
#include <string>
#include <vector>

#include <d3dx9.h>
#include <AntTweakBar.h>

#include "Defines.h"
#include "CVector3.h"
#include "Camera.h"
#include "Light.h"
#include "EntityManager.h"
#include "Messenger.h"
#include "Battle.h"
#include "tinyxml.h"
#include "attacks.h"
#include "fmod.h"
#include "fmod_event.h"

namespace gen
{

//-----------------------------------------------------------------------------
// Ant Tweak Bar
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// Game / scene constants
//-----------------------------------------------------------------------------

// Control speed
const float CameraRotSpeed = 2.0f;
float CameraMoveSpeed = 40.0f;

enum eFont { Default, HUD, End };

//-----------------------------------------------------------------------------
// Global system variables
//-----------------------------------------------------------------------------

// Get reference to global DirectX variables from another source file
extern LPDIRECT3DDEVICE9 g_pd3dDevice;
extern LPD3DXFONT        g_pFont;
extern LPD3DXFONT        g_pHUDFont;
extern LPD3DXFONT        g_pEndFont;

// Actual viewport dimensions (fullscreen or windowed)
extern TUInt32 ViewportWidth;
extern TUInt32 ViewportHeight;

// Current mouse position
extern TInt32 MouseX;
extern TInt32 MouseY;

// Messenger class for sending messages to and between entities
extern CMessenger Messenger;

//-----------------------------------------------------------------------------
// Global game/scene variables
//-----------------------------------------------------------------------------

// Effects to visualise the use of attacks and items
CAttackEffect attackEffect;
CItemEffect itemEffect;

// Entity manager
CEntityManager EntityManager;

// Lists of UIDs
TInt32 NumEnemies = 0;
TInt32 NumAllies  = 0;
vector<TEntityUID> Enemies;
vector<TEntityUID> Allies;
vector<TEntityUID> AttackOrder;
vector<CAttack> ListOfAttacks;
vector<CItem> ListOfItems;
vector<CDefence> ListOfDefence;

// Other scene elements
const int NumTrees = 200;
const int NumLights = 1;
SColourRGBA AmbientLight;
CLight* Lights[NumLights];
CCamera* MainCamera;

int numTurns = 0;

int generalAI = 1;
TInt32 NumTotal = 0;
TiXmlDocument charDoc( "Characters.xml" );
TiXmlDocument attackDoc( "Attacks.xml" );
TiXmlDocument defenceDoc( "Defences.xml" );
TiXmlDocument itemDoc( "Items.xml" );

//Variables to be used with ant Tweak bar.
extern TwBar* myBar;
extern TwBar* itemBar;

int enemyItem = 0;
int allyItem = 0;
int effectSpeedModifier = 1;

bool debugInfoOn = false; //Decide if information used when debugging is show or not
bool started = false;
bool effectOn = true;
bool templateAIOn = false;

//-----------------------------------------------------------------------------
// Game Helper functions
//-----------------------------------------------------------------------------
// Checks to see if there is anything left on the enemies side
bool EnemyAlive()
{
	TUInt32 alive = 0;	
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity( *it ))->isDead() )
		{
			alive++;
		}
	}
	if ( alive > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}
// Checks to see if there is anything left on the allies side
bool AllyAlive()
{
	TUInt32 alive = 0;
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity( *it ))->isDead() )
		{
			alive++;
		}
	}
	if ( alive > 0 )
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Returns UID for a dead Enemy or NULL if there is no dead enemy
TEntityUID DeadEnemy()
{
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() )
		{
			return *it;
		}
	}
	return NULL;
}
//Returns UID for a dead Ally or NULL if there is no dead Ally
TEntityUID DeadAlly()
{
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() )
		{
			return *it;
		}
	}
	return NULL;
}

// Selects a random enemy and returns its UID
TEntityUID RandomEnemy()
{
	bool dead = true;
	int choice = 0;
	do
	{
		choice = Random(0, Enemies.size()-1);
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(Enemies[choice]))->isDead() )
		{
			dead = true;
		}
		else
		{
			dead = false;
		}
	} while (dead);
	return Enemies[choice];
}
// Selects a random ally and returns its UID
TEntityUID RandomAlly()
{
	bool dead = true;
	int choice = 0;
	do
	{
		choice = Random(0, Allies.size()-1);
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(Allies[choice]))->isDead() )
		{
			dead = true;
		}
		else
		{
			dead = false;
		}
	} while (dead);
	return Allies[choice];
}

// Returns the UID of the enemy with the lowest health
TEntityUID LowestHealthEnemy()
{
	TEntityUID target = Enemies[0];	
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(target))->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() && 
			 static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetCurrentHealth() < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth() )
		{
			target = *it;
		}
	}
	return target;
}
// Returns the UID of the ally with the lowest health
TEntityUID LowestHealthAlly()
{
	TEntityUID target = Allies[0];	
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(target))->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() && 
			 static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetCurrentHealth() < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentHealth() )
		{
			target = *it;
		}
	}
	return target;
}

// Returns the UID of the enemy with the lowest Magic
TEntityUID LowestMagicEnemy()
{
	TEntityUID target = Enemies[0];	
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(target))->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() && 
			 static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetCurrentMagic() < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentMagic() )
		{
			target = *it;
		}
	}
	return target;
}
// Returns the UID of the ally with the lowest Magic
TEntityUID LowestMagicAlly()
{
	TEntityUID target = Allies[0];	
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( static_cast<CCharEntity*>(EntityManager.GetEntity(target))->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( !static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->isDead() && static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->GetCurrentMagic() < static_cast<CCharEntity*>(EntityManager.GetEntity(target))->GetCurrentMagic() )
		{
			target = *it;
		}
	}
	return target;
}

void SetUpAttackOrder()
{
	AttackOrder.clear();
	
	for( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		AttackOrder.push_back( *it );
	}
	for( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		AttackOrder.push_back( *it );
	}

	for( int i = 0; i < (NumTotal-1); i++ )
	{
		for ( int j = (i+1); j < (NumTotal); j++ )
		{
			TInt32 character1Speed = static_cast<CCharEntity*>(EntityManager.GetEntity( AttackOrder[i] ))->GetTemplate()->GetSpeed();
			TInt32 character2Speed = static_cast<CCharEntity*>(EntityManager.GetEntity( AttackOrder[j] ))->GetTemplate()->GetSpeed();
			TEntityUID temp = NULL;

			if ( character1Speed < character2Speed )
			{
				temp = AttackOrder[i];
				AttackOrder[i] = AttackOrder[j];
				AttackOrder[j] = temp;
			}
		}
	}
}

// Functions for getting the required variable type when reading in from an XML file.
CAttack stringToAttack ( string attack )
{
	for( int i = 0; i < static_cast<int>(ListOfAttacks.size()); i++ )
	{
		if( attack == ListOfAttacks[i].GetName() )
		{
			return ListOfAttacks[i];
		}
	}
	return ListOfAttacks[0];
}
CDefence stringToDefence ( string defence )
{
	for (int i = 0; i < static_cast<int>(ListOfAttacks.size()); i++)
	{
		if (defence== ListOfDefence[i].GetName())
		{
			return ListOfDefence[i];
		}
	}
	return ListOfDefence[0];
}
EAttackElement stringToAttackElement ( std::string element )
{
	if ( element == "Cut" )
	{
		return Cut;
	}
	else if ( element == "Crush" )
	{
		return Crush;
	}
	else if ( element == "Stab" )
	{
		return Stab;
	}
	else if ( element == "Lightning" )
	{
		return Lightning;
	}
	else if ( element == "Fire" )
	{
		return Fire;
	}
	else if ( element == "Ice" )
	{
		return Ice;
	}
	else if ( element == "Arcane" )
	{
		return Arcane;
	}
	else
	{
		return None;
	}
}
EAttackType stringToAttackType(std::string type)
{
	if(type == "Physical")
	{
		return Physical;
	}
	else if(type == "Magical")
	{
		return Magical;
	}
	else
	{
		return Both;
	}
}
EItemEffect stringToItemEffect(std::string effect)
{
	if(effect == "restoreHealth")
	{
		return restoreHealth;
	}
	else if(effect == "restoreMana")
	{
		return restoreMana;
	}
	else if(effect == "poison")
	{
		return poison;
	}
	else if(effect== "revive")
	{
		return revive;
	}
	else
	{
		return poison;
	}
	
}
EDefenceType stringToDefenceType(std::string type)
{
	if(type == "Regular")
	{
		return Regular;
	}
	else if(type == "Reflect")
	{
		return Reflect;
	}
	else if(type == "PainSplit")
	{
		return PainSplit;
	}
	else
	{
		return Regular;
	}
}

//-----------------------------------------------------------------------------
// Scene management
//-----------------------------------------------------------------------------
// Setup defence using XML
void DefenceSetup()
{
	TiXmlHandle hItemDoc(&defenceDoc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem = hItemDoc.FirstChildElement().Element();
	if(!pElem) return;
	hRoot=TiXmlHandle(pElem);

	pElem = hRoot.FirstChild("Defences").FirstChild().Element();
	if(!pElem) return;
	for(pElem; pElem; pElem=pElem->NextSiblingElement())
	{
		std::string name = pElem->Attribute("Name");
		EDefenceType type = stringToDefenceType(pElem->Attribute("Type"));
		EAttackType reciveType = stringToAttackType(pElem->Attribute("AttackReciveType"));
		EAttackElement element = stringToAttackElement(pElem->Attribute("Element"));
		TFloat32 modifier;
		pElem->QueryFloatAttribute("Modifier", &modifier);
		TInt32 cost;
		pElem->QueryIntAttribute("Cost", &cost);

		ListOfDefence.push_back(CDefence( name, type, reciveType, element, modifier, cost ));
	}
}
// Setup items using XML
void ItemSetup()
{
	TiXmlHandle hItemDoc(&itemDoc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem = hItemDoc.FirstChildElement().Element();
	if(!pElem) return;
	hRoot=TiXmlHandle(pElem);

	pElem = hRoot.FirstChild("Items").FirstChild().Element();
	if(!pElem) return;
	for(pElem; pElem; pElem=pElem->NextSiblingElement())
	{
		std::string name = pElem->Attribute("Name");
		EItemEffect effect = stringToItemEffect( pElem->Attribute("Effect"));
		int value;
		pElem->QueryIntAttribute("Value",&value);

		ListOfItems.push_back(CItem(name,effect,value));
	}
}
// Setup attacks using XML
void AttackSetup()
{
	TiXmlHandle hAttackDoc(&attackDoc);
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hAttackDoc.FirstChildElement().Element();
	if(!pElem) return;
	hRoot=TiXmlHandle(pElem);

	pElem=hRoot.FirstChild( "Attacks" ).FirstChild().Element();
	if(!pElem) return;
	for(pElem; pElem; pElem=pElem->NextSiblingElement())
	{
		std::string name = pElem->Attribute("Name");
		EAttackType type = stringToAttackType(pElem->Attribute("Type"));
		EAttackElement element = stringToAttackElement( pElem->Attribute("Element") );

		vector<SWeakness> weaknessList;
		TInt32 cost;
		TFloat32 damage,recoil;
		pElem->QueryIntAttribute("Cost",&cost);
		pElem->QueryFloatAttribute("Damage",&damage);
		pElem->QueryFloatAttribute("Recoil",&recoil);

		SWeakness weakness;
		TiXmlHandle hWeakness(0);
		hWeakness=TiXmlHandle(pElem);
		TiXmlElement* pElem2 = hWeakness.FirstChild().Element();
		for(pElem2; pElem2; pElem2=pElem2->NextSiblingElement())
		{
			weakness.element = stringToAttackElement(pElem2->Attribute("Element"));
			pElem2->QueryFloatAttribute("Modifier",&weakness.modifier);
			pElem2->QueryIntAttribute("Turns",&weakness.turns);
			weaknessList.push_back(weakness);
		}

		ListOfAttacks.push_back(CAttack(name,type,element,cost,damage,weaknessList,recoil));
	}
}
// Setup Character templates using XML
void TemplateSetup()
{
	/////////////////////////////////
	// Create character templates

	// Character templates (equivalent of meshes, but with embedded game data)
	// Template type, template name, mesh name, maximum health, Maximum magic, Strength, Inteligence, Speed, Attacks

	TiXmlHandle hCharDoc( &charDoc );
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hCharDoc.FirstChildElement().Element();
	if (!pElem) return;
	hRoot=TiXmlHandle(pElem);

	pElem=hRoot.FirstChild( "Templates" ).FirstChild().Element();
	if (!pElem) return;
	for ( pElem; pElem; pElem=pElem->NextSiblingElement() )
	{
		string type = pElem->Attribute("Type");
		string name = pElem->Attribute("Name");
		string mesh = pElem->Attribute("Mesh");

		TInt32 HP, MP, st, in, sp; 
		pElem->QueryIntAttribute("HP", &HP);
		pElem->QueryIntAttribute("MP", &MP);
		pElem->QueryIntAttribute("Strength", &st );
		pElem->QueryIntAttribute("Inteligence", &in );
		pElem->QueryIntAttribute("Speed", &sp );

		EAttackElement weak = stringToAttackElement( pElem->Attribute("Weakness") );

		TInt32 AI;
		pElem->QueryIntAttribute("AI", &AI);

		vector<CAttack> templateAttacks;

		TiXmlHandle hAttack(0);
		hAttack=TiXmlHandle(pElem);
		TiXmlElement* pElem2 = hAttack.FirstChild("Attacks").FirstChild().Element();
		for ( pElem2; pElem2; pElem2=pElem2->NextSiblingElement() )
		{
			std::string nameAttack = pElem2->Attribute("Name");
			templateAttacks.push_back( stringToAttack( nameAttack ) );
		}

		vector<CDefence> templateDefences;

		TiXmlHandle hDefence(0);
		hDefence=TiXmlHandle(pElem);
		pElem2 = hDefence.FirstChild("Defences").FirstChild().Element();
		for ( pElem2; pElem2; pElem2=pElem2->NextSiblingElement() )
		{
			std::string nameDefence = pElem2->Attribute("Name");
			templateDefences.push_back( stringToDefence( nameDefence ) );
		}

		EntityManager.CreateCharTemplate( type, name, mesh, HP, MP, st, in, sp, templateAttacks, templateDefences, weak, AI );
	}
}
// Setup Character entities using XML
void CharacterSetup()
{
	////////////////////////////////
	// Create character entities

	// Character entities (equivalent of models, but with embedded game data)
	// Template name, entity name, position, rotation

	Enemies.clear();
	Allies.clear();

	TiXmlHandle hCharDoc( &charDoc );
	TiXmlElement* pElem;
	TiXmlHandle hRoot(0);

	pElem=hCharDoc.FirstChildElement().Element();
	if (!pElem) return;
	hRoot=TiXmlHandle(pElem);

	int enemies = 0;
	int allies = 0;

	pElem=hRoot.FirstChild( "Entities" ).FirstChild().Element();
	if (!pElem) return;
	for ( pElem; pElem; pElem=pElem->NextSiblingElement() )
	{
		string type = pElem->Attribute("Type");
		string name = pElem->Attribute("Name");
		string list = pElem->Attribute("list");

		float PX, PY, PZ, RX, RY, RZ;
		TiXmlElement* pElem2 = hRoot.Child( 1 ).Child( enemies+allies ).Child( 0 ).Element();
		pElem2->QueryFloatAttribute( "X", &PX );
		pElem2->QueryFloatAttribute( "Y", &PY );
		pElem2->QueryFloatAttribute( "Z", &PZ );

		pElem2 = hRoot.Child( 1 ).Child( enemies+allies ).Child( 1 ).Element();
		pElem2->QueryFloatAttribute( "X", &RX );
		pElem2->QueryFloatAttribute( "Y", &RY );
		pElem2->QueryFloatAttribute( "Z", &RZ );

		if ( list == "Enemies" )
		{
			Enemies.push_back( EntityManager.CreateCharacter( type, name, CVector3( PX, PY, PZ ), CVector3( ToRadians(RX), ToRadians(RY), ToRadians(RZ) ) ) );
			enemies++;
		}
		else if ( list == "Allies" )
		{
			Allies.push_back( EntityManager.CreateCharacter( type, name, CVector3( PX, PY, PZ ), CVector3( ToRadians(RX), ToRadians(RY), ToRadians(RZ) ) ) );
			allies++;
		}
	}

	NumTotal = enemies + allies;

	SetUpAttackOrder();
}

// Functions for and TweakBar buttons
void TW_CALL StartRound(void* clientData)
{
	if(!started)
	{
		SMessage msg;
		msg.type = Msg_Act;
		msg.from = SystemUID;
		msg.order = 0;
		Messenger.SendMessage(AttackOrder[msg.order],msg);
		started = true;
	}
}
void TW_CALL ResetChar(void* clientData)
{
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		EntityManager.DestroyEntity( *it );
	}
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		EntityManager.DestroyEntity( *it );
	}
	CharacterSetup();
	started = false;

	attackEffect.Reset();
	itemEffect.Reset();

	numTurns = 0;
}

void TW_CALL AddPotionE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddSuperPotionE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Super Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddMagicPotionE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Magic Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddSuperMagicPotionE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Super Magic Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddVenomE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Venom" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddReviveE(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Revieve" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Enemies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}

void TW_CALL AddPotionA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddSuperPotionA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Super Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddMagicPotionA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Magic Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddSuperMagicPotionA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Super Magic Potion" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddVenomA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Venom" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}
void TW_CALL AddReviveA(void* clientData)
{
	for( auto it =  ListOfItems.begin(); it != ListOfItems.end(); it++ )
	{
		if( (*it).GetName() == "Revive" )
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity( Allies[enemyItem] ))->AddItemToInvantory( *it );
		}
	}
}

void TW_CALL InventoryRandom(void* clientData)
{
	int itemNum;

	for( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		for( auto it2 = ListOfItems.begin(); it2 != ListOfItems.end(); it2++ )
		{
			itemNum = Random(0,15);
			for(int i = 0; i < itemNum; i++)
			{
				static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(*it2);
			}
		}
		/*itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(SUPER_POTION);
		}
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(MAGIC_POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(SUPER_MAGIC_POTION);
		}
		itemNum = Random(0,10);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(VENOM);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(REVIVE);
		}*/
	}

	for(auto it = Allies.begin(); it != Allies.end(); it++)
	{
		for( auto it2 = ListOfItems.begin(); it2 != ListOfItems.end(); it2++ )
		{
			itemNum = Random(0,15);
			for(int i = 0; i < itemNum; i++)
			{
				static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(*it2);
			}
		}
		/*itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(SUPER_POTION);
		}
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(MAGIC_POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(SUPER_MAGIC_POTION);
		}
		itemNum = Random(0,10);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(VENOM);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			static_cast<CCharEntity*>(EntityManager.GetEntity(*it))->AddItemToInvantory(REVIVE);
		}*/
	}
}

//Setup ant TweakBar 
void TweakBarSetup()
{
	myBar = TwNewBar( "Tweak Bar" );
	TwDefine("'Tweak Bar' position='5 5'");
	TwAddVarRW ( myBar, "AI Used",          TW_TYPE_INT32,   &generalAI,    "min=1 max=3" );
	TwAddButton( myBar, "Start",            StartRound,      NULL,          "" );
	TwAddButton( myBar, "Reset Characters", ResetChar,       NULL,          "" );
	TwAddVarRW ( myBar, "Debug Info",       TW_TYPE_BOOLCPP, &debugInfoOn,  "" );
	TwAddVarRW ( myBar, "Attack Effect",    TW_TYPE_BOOLCPP, &effectOn,     "" );
	TwAddVarRW(myBar,"Attack Effect Speed Modifer",TW_TYPE_INT32,&effectSpeedModifier,"min=1 max=3");
	TwAddVarRW ( myBar, "Template AI",      TW_TYPE_BOOLCPP, &templateAIOn, "" );

	itemBar = TwNewBar( "Item modifier" );
	TwDefine("'Item modifier' position='205 5'");
	TwAddButton( itemBar, "Fill Inventory", InventoryRandom, NULL, "");

	TwAddSeparator(itemBar,"Enemies",NULL);

	TwAddVarRW ( itemBar, "Enemy",                    TW_TYPE_INT32,        &enemyItem, "min=0 max=4 group='Enemy Item Management'" );
	TwAddButton( itemBar, "E Add Potion",             AddPotionE,           NULL,       "group='Enemy Item Management'");
	TwAddButton( itemBar, "E Add Super Potion",       AddSuperPotionE,      NULL,       "group='Enemy Item Management'");
	TwAddButton( itemBar, "E Add Magic Potion",       AddMagicPotionE,      NULL,       "group='Enemy Item Management'");
	TwAddButton( itemBar, "E Add Super Magic Potion", AddSuperMagicPotionE, NULL,       "group='Enemy Item Management'");
	TwAddButton( itemBar, "E Add Venom",              AddVenomE,            NULL,       "group='Enemy Item Management'");
	TwAddButton( itemBar, "E Add Revive",             AddReviveE,           NULL,       "group='Enemy Item Management'");

	TwAddSeparator( itemBar, "Allies", NULL );

	TwAddVarRW ( itemBar, "Ally",                     TW_TYPE_INT32,        &allyItem, "min=0 max=3 group='Ally Item Management'" );
	TwAddButton( itemBar, "A Add Potion",             AddPotionA,           NULL,      "group='Ally Item Management'");
	TwAddButton( itemBar, "A Add Super Potion",       AddSuperPotionA,      NULL,      "group='Ally Item Management'");
	TwAddButton( itemBar, "A Add Magic Potion",       AddMagicPotionA,      NULL,      "group='Ally Item Management'");
	TwAddButton( itemBar, "A Add Super Magic Potion", AddSuperMagicPotionA, NULL,      "group='Ally Item Management'");
	TwAddButton( itemBar, "A Add Venom",              AddVenomA,            NULL,      "group='Ally Item Management'");
	TwAddButton( itemBar, "A Add Revive",             AddReviveA,           NULL,      "group='Ally Item Management'");
}

// Creates the scene geometry
bool SceneSetup()
{
	//////////////////////////////////////////
	// Create scenery templates and entities

	// Create scenery templates (equivalent of meshes)
	// Template type, template name, mesh name
	// Template type allows us to collect templates (meshes) into different categories
	EntityManager.CreateTemplate( "Scenery", "Skybox", "Skybox.x" );
	EntityManager.CreateTemplate( "Scenery", "Floor", "Floor.x" );
	EntityManager.CreateTemplate( "Scenery", "Building", "Building.x" );
	EntityManager.CreateTemplate( "Scenery", "Tree", "Tree.x" );

	EntityManager.CreateTemplate( "Effect", "Cut", "Cut.x" );
	EntityManager.CreateTemplate( "Effect", "Crush", "Crush.x");
	EntityManager.CreateTemplate( "Effect", "Stab", "Stab.x");
	EntityManager.CreateTemplate( "Effect", "Lightning", "Lightning.x");
	EntityManager.CreateTemplate( "Effect", "Fire", "Fire.x");
	EntityManager.CreateTemplate( "Effect", "Ice", "Ice.x");
	EntityManager.CreateTemplate( "Effect", "Arcane", "Arcane.x");

	EntityManager.CreateTemplate("Effect","HealthPotion","HealthPotion.x");
	EntityManager.CreateTemplate("Effect","MagicPotion","MagicPotion.x");
	EntityManager.CreateTemplate("Effect","Poison","Poison.x");
	EntityManager.CreateTemplate("Effect","Revive","Revive.x");

	vector<string> attacks;
	attacks.push_back("Cut");
	attacks.push_back("Crush");
	attacks.push_back("Stab");
	attacks.push_back("Lightning");
	attacks.push_back("Fire");
	attacks.push_back("Ice");
	attacks.push_back("Arcane");

	vector<string> items;
	items.push_back("HealthPotion");
	items.push_back("MagicPotion");
	items.push_back("Poison");
	items.push_back("Revive");

	// Creates scenery entities (equivalent of models)
	// Template name, entity name, position, rotation, scale
	// Entity name = template name if only one instance of that template (e.g. skybox)
	EntityManager.CreateEntity( "Skybox", "Skybox", CVector3(0.0f, -1000.0f, 0.0f) );
	EntityManager.CreateEntity( "Floor", "Floor" );
	EntityManager.CreateEntity( "Building", "Building0", CVector3(-70.0f, 0.0f, -60.0f),
	                                                     CVector3(0.0f, ToRadians(50.0f), 0.0f) );
	EntityManager.CreateEntity( "Building", "Building1", CVector3(60.0f, 0.0f, 80.0f) );

	for (int tree = 0; tree < NumTrees; ++tree)
	{
		EntityManager.CreateEntity( "Tree", "Tree", 
		                            CVector3(Random( -200.0f, 80.0f ), 0.0f, Random(80.0f, 200.0f)), 
									CVector3(0.0f, Random(0.0f, ToRadians(360.0f)), 0.0f),
									CVector3(1.0f, Random(0.8f, 1.2f), 1.0f) );
		EntityManager.CreateEntity( "Tree", "Tree", 
		                            CVector3(Random( -80.0f, 200.0f ), 0.0f, Random(-200.0f, -80.0f)), 
									CVector3(0.0f, Random(0.0f, ToRadians(360.0f)), 0.0f ),
									CVector3(1.0f, Random(0.8f, 1.2f), 1.0f) );
		EntityManager.CreateEntity( "Tree", "Tree", 
		                            CVector3(Random( -200.0f, -80.0f ), 0.0f, Random(-200.0f, 80.0f)), 
									CVector3(0.0f, Random(0.0f, ToRadians(360.0f)), 0.0f ),
									CVector3(1.0f, Random(0.8f, 1.2f), 1.0f) );
		EntityManager.CreateEntity( "Tree", "Tree", 
		                            CVector3(Random( 80.0f, 200.0f ), 0.0f, Random(-80.0f, 200.0f)), 
									CVector3(0.0f, Random(0.0f, ToRadians(360.0f)), 0.0f),
									CVector3(1.0f, Random(0.8f, 1.2f), 1.0f) );
	}
	attackEffect = CAttackEffect( attacks, "Attack" );
	itemEffect = CItemEffect( items, "Item" );

	attackDoc.LoadFile();
	AttackSetup();

	defenceDoc.LoadFile();
	DefenceSetup();

	itemDoc.LoadFile();
	ItemSetup();

	charDoc.LoadFile();
	TemplateSetup();
	CharacterSetup();

	TweakBarSetup();

	/////////////////////////////
	// Camera / light setup

	// Set camera position and clip planes
	MainCamera = new CCamera( CVector3( 0.0f, 5.0f, -10.0f ),
	                          CVector3(ToRadians(15.0f), 0, 0) );
	MainCamera->SetNearFarClip( 0.1f, 10000.0f ); 


	// Ambient light level
	AmbientLight = SColourRGBA( 0.6f, 0.6f, 0.6f, 1.0f );

	// Sunlight
	Lights[0] = new CLight( CVector3( -1000.0f, 800.0f, -2000.0f),
	                        SColourRGBA(1.0f, 0.9f, 0.2f), 4000.0f );

	// Set light information for render methods
	SetAmbientLight( AmbientLight );
	SetLights( &Lights[0] );

	return true;
}


// Release everything in the scene
void SceneShutdown()
{
	// Release render methods
	ReleaseMethods();

	// Release lights
	for (int light = NumLights - 1; light >= 0; --light)
	{
		delete Lights[light];
	}

	// Release camera
	delete MainCamera;

	// Destroy all entities / templates
	EntityManager.DestroyAllEntities();
	EntityManager.DestroyAllTemplates();
}


//-----------------------------------------------------------------------------
// Game loop functions
//-----------------------------------------------------------------------------

// Draw one frame of the scene
void RenderScene( float updateTime )
{
    // Begin the scene
    if (SUCCEEDED(g_pd3dDevice->BeginScene()))
    {
		// Clear the z buffer & stencil buffer - no need to clear back-buffer 
		// as the skybox will always fill the background
		g_pd3dDevice->Clear( 0, NULL, D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL, 0, 1.0f, 0 );

		// Prepare camera
		MainCamera->CalculateMatrices();

		// Render all entities
		EntityManager.RenderAllEntities( MainCamera );

		// Draw on-screen text
		RenderSceneText( updateTime );

		TwDraw();
		// End the scene
        g_pd3dDevice->EndScene();
    }

    // Present the backbuffer contents to the display
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

// Render a single text string at the given position in the given colour, may optionally centre it
void RenderText( const string& text, int X, int Y, float r, float g, float b, bool centre = false, eFont font = Default )
{
	RECT rect;
	if (!centre)
	{
		SetRect( &rect, X, Y, 0, 0 );
		switch ( font )
		{
		case Default:
			g_pFont->DrawText( NULL, text.c_str(), -1, &rect, DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		case HUD:
			g_pHUDFont->DrawText( NULL, text.c_str(), -1, &rect, DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		case End:
			g_pEndFont->DrawText( NULL, text.c_str(), -1, &rect, DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		}
	}
	else
	{
		SetRect( &rect, X - 100, Y, X + 100, 0 );
		switch ( font )
		{
		case Default:
			g_pFont->DrawText( NULL, text.c_str(), -1, &rect, 
		                       DT_CENTER | DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		case HUD:
			g_pHUDFont->DrawText( NULL, text.c_str(), -1, &rect, 
		                          DT_CENTER | DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		case End:
			g_pEndFont->DrawText( NULL, text.c_str(), -1, &rect, 
		                          DT_CENTER | DT_NOCLIP, D3DXCOLOR( r, g, b, 1.0f ) );
			break;
		}
	}
}

// Render on-screen text each frame
void RenderSceneText( float updateTime )
{
	// Write FPS text string
	stringstream outText;
	if( debugInfoOn )
	{
		outText << "Frame Time: " << updateTime * 1000.0f << "ms" << endl << "FPS:" << 1.0f / updateTime;
		RenderText(outText.str(),0,0,1.0f,1.0f,0.0f);
		outText.str("");

		CVector3 TargetPos = attackEffect.getTargetPos();

		outText << "Attack Effect State: " << attackEffect.getState() << std::endl
			    << "Item Effect State: " << itemEffect.getState();
		RenderText(outText.str(),ViewportWidth/2,0,1.0f,1.0f,1.0f,true,End);
		outText.str("");
	}

	int count = 0;

	// Write name next to each Ally
	for ( auto it = Allies.begin(); it != Allies.end(); ++it)
	{
		// Get car entity, then world position
		CCharEntity* allyEntity = static_cast<CCharEntity*>(EntityManager.GetEntity( *it ));
		CVector3 allyPt = allyEntity->Position();

		// Convert car world position to pixel coordinate (picking in Camera class)
		int X, Y;
		if (MainCamera->PixelFromWorldPt( allyPt, ViewportWidth, ViewportHeight, &X, &Y ))
		{
			if ( debugInfoOn )
			{
				RenderText( "Attack: "  + allyEntity->GetAttackName(),  X, Y,    0.6f, 1.0f, 0.6f, true );
				RenderText( "Defence: " + allyEntity->GetDefenceInfo(),    X, Y+10, 0.6f, 1.0f, 0.6f, true );
			}
		}
		int HUDX = ViewportWidth - (ViewportWidth / 3);
		int HUDY = ViewportHeight - (ViewportHeight / 5) + (ViewportHeight / 25) * count;
		int displayHealth = allyEntity->GetCurrentHealth();
		if(displayHealth < 0)
		{
			displayHealth = 0;
		}
		int displayMagic = allyEntity->GetCurrentMagic();
		if(displayMagic < 0)
		{
			displayMagic = 0;
		}
		outText << allyEntity->Template()->GetName().c_str() << " "
			    << allyEntity->GetName().c_str()
			    << "  |  HP: " << displayHealth << "/"
			    << allyEntity->GetTemplate()->GetMaxHealth()
			    << "  |  MP: " << displayMagic << "/"
			    << allyEntity->GetTemplate()->GetMaxMagic();
		RenderText(outText.str(),HUDX,HUDY,1.0f,1.0f,1.0f,false,HUD);
		outText.str("");

		count++;
	}

	count = 0;
	// Write name next to each enemy
	for ( auto it = Enemies.begin(); it != Enemies.end(); ++it)
	{
		// Get car entity, then world position
		CCharEntity* enemyEntity = static_cast<CCharEntity*>(EntityManager.GetEntity( *it ));
		CVector3 enemyPt = enemyEntity->Position();

		// Convert car world position to pixel coordinate (picking in Camera class)
		int X, Y;
		if (MainCamera->PixelFromWorldPt( enemyPt, ViewportWidth, ViewportHeight, &X, &Y ))
		{
			if( debugInfoOn )
			{
				RenderText( "Attack: "  + enemyEntity->GetAttackName(),  X, Y,    0.6f, 1.0f, 0.6f, true );
				RenderText( "Defence: " + enemyEntity->GetDefenceInfo(),    X, Y+10, 0.6f, 1.0f, 0.6f, true );
			}
		}
		int HUDX = 0 + (ViewportWidth / 10);
		int HUDY = ViewportHeight - (ViewportHeight / 5) + (ViewportHeight / 25) * count;
		int displayHealth = enemyEntity->GetCurrentHealth();
		if(displayHealth < 0)
		{
			displayHealth = 0;
		}
		int displayMagic = enemyEntity->GetCurrentMagic();
		if(displayMagic < 0)
		{
			displayMagic = 0;
		}
		outText << enemyEntity->Template()->GetName().c_str() << " "
			    << enemyEntity->GetName().c_str()
			    << "  |  HP: " << displayHealth << "/"
			    << enemyEntity->GetTemplate()->GetMaxHealth()
			    << "  |  MP: " << displayMagic << "/"
			    << enemyEntity->GetTemplate()->GetMaxMagic();
		RenderText(outText.str(),HUDX,HUDY,1.0f,1.0f,1.0f,false,HUD);
		outText.str("");

		count++;
	}

	if ( !EnemyAlive() )
	{
		outText << "Allies Win" << std::endl 
			    << "Number of turns in battle: " << numTurns;
	}
	else if ( !AllyAlive() )
	{
		outText << "Enemies Win" << std::endl 
			    << "Number of turns in battle: " << numTurns;
	}
	else
	{
		outText.str("");
	}
	RenderText( outText.str(), ViewportWidth/2, ViewportHeight/2, 1.0f, 1.0f, 0.0f, true, End );
	outText.str("");
}

// Update the scene between rendering
void UpdateScene( float updateTime )
{
	static int attackOrderPos;
	// Call all entity update functions
	EntityManager.UpdateAllEntities( updateTime );
	attackEffect.Update( updateTime * effectSpeedModifier );
	itemEffect.Update( updateTime * effectSpeedModifier );

	// Key F1 used for full screen toggle
	// System messages
	// Go
	if (KeyHit( Key_Return ) && !started)
	{
		SMessage msg;
		msg.type = Msg_Act;
		msg.from = SystemUID;
		Messenger.SendMessage( AttackOrder[attackOrderPos], msg );
		started = true;
	}

	if (KeyHit( Key_R ))
	{
		for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
		{
			EntityManager.DestroyEntity( *it );
		}
		for ( auto it = Allies.begin(); it != Allies.end(); it++ )
		{
			EntityManager.DestroyEntity( *it );
		}
		CharacterSetup();

		started = false;
		attackEffect.Reset();
		itemEffect.Reset();

		numTurns = 0;
	}

	if (KeyHit( Key_1 ))
	{
		generalAI = 1;
	}
	if (KeyHit( Key_2 ))
	{
		generalAI = 2;
	}
	if (KeyHit( Key_3 ))
	{
		generalAI = 3;
	}

	if(!AllyAlive() || !EnemyAlive())
	{
		SMessage msg;
		msg.type = Msg_Stop;
		for(auto it = AttackOrder.begin(); it != AttackOrder.end(); it++)
		{
			Messenger.SendMessage( *it, msg );
		}
	}

	if( started )
	{
		if( attackEffect.getState() == Inactive && itemEffect.getState() == InactiveI )
		{
			SMessage msg;
			msg.type = Msg_Act;
			attackOrderPos++;
			if(attackOrderPos >= NumTotal)
			{
				attackOrderPos = 0;
			}
			Messenger.SendMessage(AttackOrder[attackOrderPos],msg);
		}
	}
	else
	{
		attackOrderPos = 0;
	}

	// Move the camera
	MainCamera->Control( Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D, 
	                     CameraMoveSpeed * updateTime, CameraRotSpeed * updateTime );
}

} // namespace gen
