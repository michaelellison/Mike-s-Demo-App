/// \file CATVariant.h
/// \brief Variant type for databases
/// \ingroup CAT
///
/// Copyright (c) 2010 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
#ifndef _CATVariant_H_
#define _CATVariant_H_
#include "CATInternal.h"
#include "CATString.h"

typedef enum CATVARIANT_TYPE
{
    CATVARIANT_NULL,
    CATVARIANT_INT64,
    CATVARIANT_DOUBLE,
    CATVARIANT_TEXT,
    //----------------
    CATVAIRANT_NUMTYPES
};

class CATVariant
{
    public:
        CATVariant();
        CATVariant( const CATVariant&   src);
        CATVariant( const char*         val);
        CATVariant( CATInt64            val);
        CATVariant( CATFloat64          val);
        
        ~CATVariant();


        CATVARIANT_TYPE GetType() const;

        // return true if changed.
        bool        Clear       ();

        bool        SetInt64    ( CATInt64          val);
        bool        SetDouble   ( CATFloat64        val);
        bool        SetString   ( const CATString&  val);

        CATInt64    GetInt64    () const;
        CATFloat64  GetDouble   () const;
        CATString   GetString   () const;

        CATVariant& operator=   ( const CATVariant& src);
        CATVariant& operator=   ( CATFloat64        val);
        CATVariant& operator=   ( CATInt64          val);
        CATVariant& operator=   ( const char*       val);

        bool        operator==  ( const CATVariant& cmp) const;

    protected:
        CATVARIANT_TYPE     fType;
        CATString           fString;
        CATInt64            fInt64;
        CATFloat64          fDouble;
};

#endif // _CATVariant_H_