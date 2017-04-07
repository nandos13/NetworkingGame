#include "ShootAction.h"



#ifndef NETWORK_SERVER
// Client-side execution
void ShootAction::_Execute(float dTime)
{
	// Slow action time-scale for slow-mo effect while firing happens
	SlowTime(0.35f);

	// TODO: Find a way to wait for a moment before firing, like the animation plays
	// in XCOM. Maybe use a thread here.

	// TODO: Shoot enemy
	// Query victim armour, etc before applying damage.
}
#endif

#ifdef NETWORK_SERVER
// Server-side execution
void ShootAction::_Execute(float dTime)
{
	// TODO
}
#endif

ShootAction::ShootAction(Character * owner, MapVec3 target, short damage, bool crit) : BaseAction(owner)
{
	m_target = target;
	m_damage = damage;
	m_crit = crit;
}


ShootAction::~ShootAction()
{
}
