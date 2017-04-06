#include "ShootAction.h"



void ShootAction::_Execute(float dTime)
{
	if (m_owner != nullptr)
	{
		// TODO: Shoot enemy
	}
	else
		NoOwnerError();
}

ShootAction::ShootAction(Character * owner, MapVec3 target, short damage) : GameAction(owner)
{
	m_target = target;
	m_damage = damage;
}


ShootAction::~ShootAction()
{
}
