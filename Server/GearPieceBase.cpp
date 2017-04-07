#include "GearPieceBase.h"



GearPieceBase::GearPieceBase(char* name)
{
	m_name = name;
}

GearPieceBase::~GearPieceBase()
{
}

const char * GearPieceBase::GetName()
{
	return m_name;
}
