#pragma once
class Character
{
private:
	// Base Stats
	unsigned int m_baseHealth;
	unsigned int m_baseAim;
	unsigned int m_baseMobility;
	unsigned int m_baseDefense;
	unsigned int m_baseCritChance;

	const unsigned int m_sightRadius = 27;

	// Variable Stats


public:
	Character();
	~Character();
};

