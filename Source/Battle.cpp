/*******************************************
	CarTag.cpp

	Shell scene and game functions
********************************************/

#include <sstream>
#include <string>
#include <vector>
using namespace std;

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

CAttackEffect attackEffect;

// Entity manager
CEntityManager EntityManager;

// Lists of UIDs
TInt32 NumEnemies = 0;
TInt32 NumAllies  = 0;
vector<TEntityUID> Enemies;
vector<TEntityUID> Allies;
vector<TEntityUID> AttackOrder;

// Other scene elements
const int NumTrees = 200;
const int NumLights = 1;
SColourRGBA AmbientLight;
CLight* Lights[NumLights];
CCamera* MainCamera;

int generalAI = 1;
TInt32 NumTotal = 0;
TiXmlDocument charDoc( "Characters.xml" );

//Variables to be used with ant Tweak bar.
extern TwBar* myBar;
extern TwBar* itemBar;

int enemyItem = 0;
int allyItem = 0;

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
		if ( !EntityManager.GetCharEntity( *it )->isDead() )
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
		if ( !EntityManager.GetCharEntity( *it )->isDead() )
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
		if ( EntityManager.GetCharEntity(*it)->isDead() )
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
		if ( EntityManager.GetCharEntity(*it)->isDead() )
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
		if ( EntityManager.GetCharEntity(Enemies[choice])->isDead() )
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
		if ( EntityManager.GetCharEntity(Allies[choice])->isDead() )
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
		if ( EntityManager.GetCharEntity(target)->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( !EntityManager.GetCharEntity(*it)->isDead() && EntityManager.GetCharEntity(*it)->GetCurrentHealth() < EntityManager.GetCharEntity(target)->GetCurrentHealth() )
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
		if ( EntityManager.GetCharEntity(target)->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( !EntityManager.GetCharEntity(*it)->isDead() && EntityManager.GetCharEntity(*it)->GetCurrentHealth() < EntityManager.GetCharEntity(target)->GetCurrentHealth() )
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
		if ( EntityManager.GetCharEntity(target)->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		if ( !EntityManager.GetCharEntity(*it)->isDead() && EntityManager.GetCharEntity(*it)->GetCurrentMagic() < EntityManager.GetCharEntity(target)->GetCurrentMagic() )
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
		if ( EntityManager.GetCharEntity(target)->isDead() )
		{
			target = *it;
		}
	}
	for ( auto it = Allies.begin(); it != Allies.end(); it++ )
	{
		if ( !EntityManager.GetCharEntity(*it)->isDead() && EntityManager.GetCharEntity(*it)->GetCurrentMagic() < EntityManager.GetCharEntity(target)->GetCurrentMagic() )
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
			TInt32 character1Speed = EntityManager.GetCharEntity( AttackOrder[i] )->GetTemplate()->GetSpeed();
			TInt32 character2Speed = EntityManager.GetCharEntity( AttackOrder[j] )->GetTemplate()->GetSpeed();
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
//void CleanAttackOrder()
//{
//	vector<int> deadPos;
//	for(int i = 0; i < AttackOrder.size(); i++)
//	{
//		if(EntityManager.GetCharEntity(AttackOrder[i])->isDead())
//		{
//			deadPos.push_back(i);
//		}
//	}
//
//	for(int i =0; i < deadPos.size(); i++)
//	{
//		//EntityManager.DestroyEntity(AttackOrder[deadPos[i]]);
//		AttackOrder.erase(AttackOrder.begin()+deadPos[i]);
//		NumTotal--;
//	}
//}

// Functions for getting the required variable type when reading in from an XML file.
SAttack stringToAttack ( string attack )
{
	if ( attack == "CUT" )
	{
		 return CUT;
	}
	else if ( attack == "CRUSH" )
	{
		return CRUSH;
	}
	else if ( attack == "STAB" )
	{
		return STAB;
	}
	else if ( attack == "LIGHTNING" )
	{
		return LIGHTNING;
	}
	else if ( attack == "FIRE" )
	{
		return FIRE;
	}
	else if ( attack == "ICE" )
	{
		return ICE;
	}
	else if ( attack == "ARCANE" )
	{
		return ARCANE;
	}
	else
	{
		return CUT;
	}
}
SDefence stringToDefence ( string defence )
{
	if ( defence == "BASIC_P" )
	{
		return BASIC_PHYSICAL;
	}
	else if ( defence == "BASIC_M" )
	{
		return BASIC_MAGICAL;
	}
	else if ( defence == "ADV_P" )
	{
		return ADVANCED_PHYSICAL;
	}
	else if ( defence == "ADV_M" )
	{
		return ADVANCED_MAGICAL;
	}
	else if ( defence == "LIGHT" )
	{
		return LIGHTNING_DEFENCE;
	}
	else if ( defence == "FIRE" )
	{
		return FIRE_DEFENCE;
	}
	else if ( defence == "ICE" )
	{
		return ICE_DEFENCE;
	}
	else if ( defence == "ARCANE" )
	{
		return ARCANE_DEFENCE;
	}
	else if ( defence == "REFLECT" )
	{
		return REFLECT;
	}
	else if ( defence == "PAIN_SPLIT" )
	{
		return PAIN_SPLIT;
	}
	else
	{
		return BASIC_PHYSICAL;
	}
}
EElement stringToWeakness ( string weakness )
{
	if ( weakness == "Cut" )
	{
		return Cut;
	}
	else if ( weakness == "Crush" )
	{
		return Crush;
	}
	else if ( weakness == "Stab" )
	{
		return Stab;
	}
	else if ( weakness == "Lightning" )
	{
		return Lightning;
	}
	else if ( weakness == "Fire" )
	{
		return Fire;
	}
	else if ( weakness == "Ice" )
	{
		return Ice;
	}
	else if ( weakness == "Arcane" )
	{
		return Arcane;
	}
	else
	{
		return None;
	}
}

//-----------------------------------------------------------------------------
// Scene management
//-----------------------------------------------------------------------------
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

		vector<SAttack> templateAttacks;
		templateAttacks.push_back( stringToAttack( pElem->Attribute("Attack1") ) );
		templateAttacks.push_back( stringToAttack( pElem->Attribute("Attack2") ) );
		templateAttacks.push_back( stringToAttack( pElem->Attribute("Attack3") ) );
		templateAttacks.push_back( stringToAttack( pElem->Attribute("Attack4") ) );

		vector<SDefence> templateDefences;
		templateDefences.push_back( stringToDefence( pElem->Attribute("Defence1") ) );
		templateDefences.push_back( stringToDefence( pElem->Attribute("Defence2") ) );
		templateDefences.push_back( stringToDefence( pElem->Attribute("Defence3") ) );
		templateDefences.push_back( stringToDefence( pElem->Attribute("Defence4") ) );

		EElement weak = stringToWeakness( pElem->Attribute("Weakness") );

		TInt32 AI;
		pElem->QueryIntAttribute("AI", &AI);

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

//Setup ant TweakBar
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
}

void TW_CALL AddPotionE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( POTION );
}
void TW_CALL AddSuperPotionE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( SUPER_POTION );
}
void TW_CALL AddMagicPotionE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( MAGIC_POTION );
}
void TW_CALL AddSuperMagicPotionE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( SUPER_MAGIC_POTION );
}
void TW_CALL AddVenomE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( VENOM );
}
void TW_CALL AddReviveE(void* clientData)
{
	EntityManager.GetCharEntity( Enemies[enemyItem] )->AddItemToInvantory( REVIVE );
}

void TW_CALL AddPotionA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(POTION);
}
void TW_CALL AddSuperPotionA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(SUPER_POTION);
}
void TW_CALL AddMagicPotionA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(MAGIC_POTION);
}
void TW_CALL AddSuperMagicPotionA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(SUPER_MAGIC_POTION);
}
void TW_CALL AddVenomA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(VENOM);
}
void TW_CALL AddReviveA(void* clientData)
{
	EntityManager.GetCharEntity(Allies[allyItem])->AddItemToInvantory(REVIVE);
}

void TW_CALL InventoryRandom(void* clientData)
{
	int itemNum;

	for( auto it = Enemies.begin(); it != Enemies.end(); it++ )
	{
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(SUPER_POTION);
		}
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(MAGIC_POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(SUPER_MAGIC_POTION);
		}
		itemNum = Random(0,10);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(VENOM);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(REVIVE);
		}
	}

	for(auto it = Allies.begin(); it != Allies.end(); it++)
	{
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(SUPER_POTION);
		}
		itemNum = Random(0,15);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(MAGIC_POTION);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(SUPER_MAGIC_POTION);
		}
		itemNum = Random(0,10);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(VENOM);
		}
		itemNum = Random(0,5);
		for(int i = 0; i < itemNum; i++)
		{
			EntityManager.GetCharEntity(*it)->AddItemToInvantory(REVIVE);
		}
	}
}

void TweakBarSetup()
{
	myBar = TwNewBar( "Tweak Bar" );
	TwAddVarRW ( myBar, "AI Used",          TW_TYPE_INT32,   &generalAI,    "min=1 max=3" );
	TwAddButton( myBar, "Start",            StartRound,      NULL,          "" );
	TwAddButton( myBar, "Reset Characters", ResetChar,       NULL,          "" );
	TwAddVarRW ( myBar, "Debug Info",       TW_TYPE_BOOLCPP, &debugInfoOn,  "" );
	TwAddVarRW ( myBar, "Attack Effect",    TW_TYPE_BOOLCPP, &effectOn,     "" );
	TwAddVarRW ( myBar, "Template AI",      TW_TYPE_BOOLCPP, &templateAIOn, "" );

	itemBar = TwNewBar( "Item modifier" );
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
	EntityManager.CreateTemplate( "Effect", "Attack", "Sphere.x" );

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
	attackEffect = CAttackEffect( "Attack", "Attack" );

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
	}

	int count = 0;

	// Write name next to each Ally
	for ( auto it = Allies.begin(); it != Allies.end(); ++it)
	{
		// Get car entity, then world position
		CCharEntity* allyEntity = EntityManager.GetCharEntity( *it );
		CVector3 allyPt = allyEntity->Position();

		// Convert car world position to pixel coordinate (picking in Camera class)
		int X, Y;
		if (MainCamera->PixelFromWorldPt( allyPt, ViewportWidth, ViewportHeight, &X, &Y ))
		{
			if ( debugInfoOn )
			{
				RenderText( "Attack: "  + allyEntity->GetAttackElement(),  X, Y,    0.6f, 1.0f, 0.6f, true );
				RenderText( "Defence: " + allyEntity->GetDefenceInfo(),    X, Y+10, 0.6f, 1.0f, 0.6f, true );
				//RenderText( "Items: "   + allyEntity->GetNumInInvantory(), X, Y+20, 0.6f, 1.0f, 0.6f, true );
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
		CCharEntity* enemyEntity = EntityManager.GetCharEntity( *it );
		CVector3 enemyPt = enemyEntity->Position();

		// Convert car world position to pixel coordinate (picking in Camera class)
		int X, Y;
		if (MainCamera->PixelFromWorldPt( enemyPt, ViewportWidth, ViewportHeight, &X, &Y ))
		{
			if( debugInfoOn )
			{
				RenderText( "Attack: "  + enemyEntity->GetAttackElement(),  X, Y,    0.6f, 1.0f, 0.6f, true );
				RenderText( "Defence: " + enemyEntity->GetDefenceInfo(),    X, Y+10, 0.6f, 1.0f, 0.6f, true );
				RenderText( "Items: "   + enemyEntity->GetNumInInvantory(), X, Y+20, 0.6f, 1.0f, 0.6f, true );
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
		outText << "Allies Win";
	}
	else if ( !AllyAlive() )
	{
		outText << "Enemies Win";
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

	// Call all entity update functions
	EntityManager.UpdateAllEntities( updateTime );
	attackEffect.Update( updateTime );

	// Key F1 used for full screen toggle
	// System messages
	// Go
	if (KeyHit( Key_Return ) && !started)
	{
		SMessage msg;
		msg.type = Msg_Act;
		msg.from = SystemUID;
		msg.order = 0;
		Messenger.SendMessage( AttackOrder[msg.order], msg );
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

	if(KeyHit(Key_0))
	{
		SMessage msg;
		msg.type = Msg_Poison;
		Messenger.SendMessage(Enemies[enemyItem],msg);
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

	// Move the camera
	//MainCamera->Control( Key_Up, Key_Down, Key_Left, Key_Right, Key_W, Key_S, Key_A, Key_D, 
	//                     CameraMoveSpeed * updateTime, CameraRotSpeed * updateTime );
}

} // namespace gen
