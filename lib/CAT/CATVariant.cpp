/// \file CATVariant.cpp
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
#include "CATVariant.h"

CATVariant::CATVariant()
{
    fType   = CATVARIANT_NULL;
    fInt64  = 0;
    fDouble = 0.0;
}

CATVariant::~CATVariant()
{
}

CATVariant::CATVariant(const CATVariant& src)
{
    fType   = CATVARIANT_NULL;
    fInt64  = 0;
    fDouble = 0.0;

    *this = src;
}

CATVariant& CATVariant::operator=(const CATVariant& src)
{
    if (&src == this)
        return *this;

    switch (src.fType)
    {
        case CATVARIANT_INT64:
            this->SetInt64(src.fInt64);
            break;
        case CATVARIANT_DOUBLE:
            this->SetDouble(src.fDouble);
            break;
        case CATVARIANT_TEXT:
            this->SetString(src.fString);
            break;
        default:
        case CATVARIANT_NULL:
            Clear();
            break;
    }

    return *this;
}

bool CATVariant::operator==(const CATVariant& cmp) const
{
    if (this->fType != cmp.fType)
        return false;
    if (&cmp == this)
        return true;

    switch (this->fType)
    {
        case CATVARIANT_NULL:
            return true;
            break;
        case CATVARIANT_INT64:
            if (this->fInt64 == cmp.fInt64)
                return true;
            break;
        case CATVARIANT_DOUBLE:
            if (this->fDouble == cmp.fDouble)
                return true;
            break;
        case CATVARIANT_TEXT:
            if (this->fString.Compare(cmp.fString) == 0)
                return true;
            break;
    }

    return false;
}

bool CATVariant::Clear()
{
    bool retVal = (fType != CATVARIANT_NULL);
    fType   = CATVARIANT_NULL;
    fInt64  = 0;
    fDouble = 0.0;
    fString = "";
    return retVal;
}

CATVARIANT_TYPE CATVariant::GetType() const
{
    return fType;
}

bool CATVariant::SetInt64(CATInt64 val)
{
    bool retVal = (fType != CATVARIANT_INT64) || (fInt64 != val);
    fInt64 = val;
    fType = CATVARIANT_INT64;
    return retVal;
}

bool CATVariant::SetDouble(CATFloat64 val)
{
    bool retVal = (fType != CATVARIANT_DOUBLE) || (fDouble != val);
    fDouble = val;
    fType = CATVARIANT_DOUBLE;
    return retVal;
}

bool CATVariant::SetString(const CATString& val)
{
    bool retVal = (fType != CATVARIANT_TEXT) || (fString.Compare(val) != 0);
    fString = val;
    fType = CATVARIANT_TEXT;
    return retVal;
}


CATInt64 CATVariant::GetInt64() const
{
    CATInt64 val = 0;
    switch (fType)
    {
        case CATVARIANT_NULL:
            break;
        case CATVARIANT_DOUBLE:
            val = (CATInt64)fDouble;
            break;
        case CATVARIANT_TEXT:
            val = (CATInt64)fString;
            break;
        case CATVARIANT_INT64:
            val = fInt64;
            break;
    }            
    return val;
}

CATFloat64 CATVariant::GetDouble() const
{
    CATFloat64 val = 0.0;
    switch (fType)
    {
        case CATVARIANT_NULL:
            break;
        case CATVARIANT_DOUBLE:
            val = fDouble;
            break;
        case CATVARIANT_TEXT:
            val = (CATFloat64)fString;
            break;
        case CATVARIANT_INT64:
            val = (CATFloat64)fInt64;
            break;
    }            
    return val;
}

CATString CATVariant::GetString() const
{
    CATString val;
    switch (fType)
    {
        case CATVARIANT_NULL:
            break;
        case CATVARIANT_DOUBLE:
            val << fDouble;
            break;
        case CATVARIANT_TEXT:
            val = fString;
            break;
        case CATVARIANT_INT64:
            val << fInt64;
            break;
    }            
    return val;
}

CATVariant::CATVariant(CATFloat64 val)
{
    fType   = CATVARIANT_NULL;
    SetDouble(val);    
}

CATVariant::CATVariant(CATInt64 val)
{
    fType   = CATVARIANT_NULL;
    SetInt64(val);    
}
CATVariant::CATVariant(const char* val)
{
    fType   = CATVARIANT_NULL;
    SetString(val);
}

CATVariant& CATVariant::operator=(CATFloat64 val)
{
    SetDouble(val);
    return *this;
}
CATVariant& CATVariant::operator=(CATInt64 val)
{
    SetInt64(val);
    return *this;
}
CATVariant& CATVariant::operator=(const char* val)
{
    SetString(val);
    return *this;
}
