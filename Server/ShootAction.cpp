#include "ShootAction.h"
#include "Character.h"
#include "Game.h"



#ifndef NETWORK_SERVER
// Client-side execution
void ShootAction::_Execute(float dTime)
{
	// Slow action time-scale for slow-mo effect while firing happens
	SlowTime(0.35f);

	// TODO: Find a way to wait for a moment before firing, like the animation plays
	// in XCOM. Maybe use a thread here.

	// TODO: Check if visual stuff is done before running following code:

	// Use ammo
	m_owner->UseAmmo((unsigned int)m_ammo);

	// Apply damage
	Character* c = Game::GetInstance()->FindCharacterAtCoords(m_target);
	if (c != nullptr)
		c->ApplyDamage(m_damage);

	// End action
	CompleteSelf();
}
#endif

#ifdef NETWORK_SERVER
// Server-side execution
void ShootAction::_Execute(float dTime)
{
	// Use ammo
	m_owner->UseAmmo( (unsigned int)m_ammo );

	// Deal damage
	Character* c = Game::GetInstance()->FindCharacterAtCoords(m_target);
	if (c != nullptr)
		c->ApplyDamage(m_damage);

	// End action
	CompleteSelf();
}
#endif

ShootAction::ShootAction(Character * owner, MapVec3 target, short damage, unsigned int ammoUse, bool armourShred) : BaseAction(owner)
{
	m_target = target;
	m_damage = damage;
	m_shred = armourShred;
	m_ammo = ammoUse;
}

ShootAction::~ShootAction()
{
}

#ifndef NETWORK_SERVER
ShootAction* ShootAction::Read(RakNet::BitStream & bsIn)
{
	// Read info
	short characterID = 0;
	MapVec3 target = MapVec3(0);
	unsigned int ammo = 0;
	short damage = 0;
	bool shred = 0;

	bsIn.Read(characterID);
	bsIn.Read(target);
	bsIn.Read(ammo);
	bsIn.Read(damage);
	bsIn.Read(shred);

	// Find character by ID
	Character* c = Game::GetInstance()->FindCharacterByID(characterID);

	// Error check
	if (c == nullptr)
	{
		printf("Error: Could not find character with id: %d\n", characterID);
		return nullptr;
	}

	// Create & return action
	ShootAction* sA = new ShootAction(c, target, damage, ammo, shred);
	return sA;
}
#endif

#ifdef NETWORK_SERVER
void ShootAction::Write(RakNet::BitStream & bs)
{
	// Write character index
	bs.Write(m_owner->GetID());

	// Write target tile
	m_target.Write(bs);

	// Write damage, ammo, etc
	bs.Write(m_ammo);
	bs.Write(m_damage);
	bs.Write(m_shred);
}
#endif
