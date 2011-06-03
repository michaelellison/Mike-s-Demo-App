/// \class CATFilterAttackDecay.cpp
/// \brief Attack/Decay filter
/// \ingroup CAT
///
/// f(x) = coef * f(x-1) + (1-coef) * x, with seperate rising/falling coefficients for attack/decay.
/// First value is now filtered against 0.

#include "CATFilterAttackDecay.h"

// ------------------------------------------------------------------
// CATFilterAttackDecay() constructor
//
// \param attack - attack coefficient
// \param decay - decay coefficient
// ------------------------------------------------------------------
CATFilterAttackDecay::CATFilterAttackDecay(CATFloat32 attack, CATFloat32 decay) 
: CATFilter()
{
	if (attack > .999f)
	{
		attack = .999f;
	}

	if (decay > .999f)
	{
		decay = .999f;
	}

	fAttack = attack;
	fDecay  = decay;
}

CATFilterAttackDecay::~CATFilterAttackDecay()
{
	
}
		

// ------------------------------------------------------------------
// SetAttack() sets the attack coefficient
//
// \param attack - attack coefficient
// \sa SetDecay()      
// ------------------------------------------------------------------
void CATFilterAttackDecay::SetAttack(CATFloat32 attack)
{
	if (attack > .999f)
	{
		attack = .999f;
	}
	fAttack = attack;
}

// ------------------------------------------------------------------
// SetDecay() sets the decay coefficient
//
// \param decay - decay coefficient
// \sa SetAttack()      
// ------------------------------------------------------------------
void CATFilterAttackDecay::SetDecay(CATFloat32 decay)
{
	if (decay > .999f)
	{
		decay = .999f;
	}
	fDecay = decay;
}

// ------------------------------------------------------------------
// GetAttack() retrieves the attack coefficient
//
// \return CATFloat32 - attack coefficient
// ------------------------------------------------------------------
CATFloat32 CATFilterAttackDecay::GetAttack()
{
	return fAttack;
}

// ------------------------------------------------------------------
// GetDecay() retrieves the decay coefficient
//
// \return CATFloat32 - decay coefficient
// ------------------------------------------------------------------
CATFloat32 CATFilterAttackDecay::GetDecay()
{
	return fDecay;
}
