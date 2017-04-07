#pragma once
class GearPieceBase
{
protected:
	// Modifiers
	int m_health;
	int m_aim;
	int m_mobility;
	int m_defense;
	int m_critChance;
	char* m_name;

public:
	GearPieceBase(char* name);
	~GearPieceBase();

	const char* GetName();
};

