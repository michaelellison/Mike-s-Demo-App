//---------------------------------------------------------------------------
/// \file CATFilterAttackDecay.h
/// \brief Attack/Decay filter
/// \ingroup CAT
/// 
/// Copyright (c) 2002-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------
#ifndef _CATFilterAttackDecay_H_
#define _CATFilterAttackDecay_H_

#include "CATFilter.h"

/// \class CATFilterAttackDecay
/// \brief Attack/Decay filter
/// \ingroup CAT
///
/// f(x) = coef * f(x-1) + (1-coef) * x, with seperate rising/falling coefficients for attack/decay.
/// First value is now filtered against 0.
class CATFilterAttackDecay : public CATFilter
{
	public:
      /// CATFilterAttackDecay() constructor
      ///
      /// \param attack - attack coefficient
      /// \param decay - decay coefficient
		CATFilterAttackDecay(CATFloat32 attack, CATFloat32 decay);

		virtual ~CATFilterAttackDecay();
		
      /// SetAttack() sets the attack coefficient
      ///
      /// \param attack - attack coefficient
      /// \sa SetDecay()      
		virtual	void	  SetAttack( CATFloat32 attack);

      /// SetDecay() sets the decay coefficient
      ///
      /// \param decay - decay coefficient
      /// \sa SetAttack()      
		virtual	void	  SetDecay( CATFloat32 decay);

      /// GetAttack() retrieves the attack coefficient
      ///
      /// \return CATFloat32 - attack coefficient
		virtual  CATFloat32 GetAttack();

      /// GetDecay() retrieves the decay coefficient
      ///
      /// \return CATFloat32 - decay coefficient
		virtual  CATFloat32 GetDecay();

		inline CATFloat32 Filter(CATFloat32 rawVal)
		{
			if (rawVal < fLastFiltered)
			{
				fLastFiltered =  CATTruncDenormals((fDecay * fLastFiltered) + (1.0f - fDecay) * rawVal);	
			}
			else
			{
				fLastFiltered =  CATTruncDenormals((fAttack * fLastFiltered) + (1.0f - fAttack) * rawVal);	
			}

			return fLastFiltered;
		}
	protected:

		CATFloat32	fAttack;
		CATFloat32  fDecay;
};


#endif // _CATFilterAttackDecay_H_
