//---------------------------------------------------------------------------
/// \file CATPrefs.cpp
/// \brief Preference storage class
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//---------------------------------------------------------------------------

#include "CATPrefs.h"
#include "CATXMLFactory.h"
#include "CATXMLParser.h"
#include "CATFileSystem.h"
#include "CATApp.h"
#include "CATEventDefs.h"

//---------------------------------------------------------------------------
// Constructor - just store the prefs name
//---------------------------------------------------------------------------
CATPrefs::CATPrefs(const CATString& prefFile)
{
    fRootNode = 0;
    fPrefFile = prefFile;
    fRootNode = new CATXMLObject(L"Preferences");
}

//---------------------------------------------------------------------------
// Destructor - clear prefs lists
//---------------------------------------------------------------------------
CATPrefs::~CATPrefs()
{   
    Clear();   
}

//---------------------------------------------------------------------------
// Clear prefs - clean up any prefs in our lists 
//---------------------------------------------------------------------------
void CATPrefs::Clear()
{
    this->fPrefLock.Wait();

    if (fRootNode)
    {
        delete fRootNode;
        fRootNode = 0;
    }

    this->fPrefLock.Release();
}

//---------------------------------------------------------------------------
// Note: the GetPref / Save pref simply get and save from and to memory.
//       you must call Load/Save to access the actual file.  
//       To revert, call load again w/o saving
// Retrieve the preference of whatever type
//---------------------------------------------------------------------------
bool CATPrefs::GetPref(  const CATString& prefSection, 
                         const CATString& prefName, 
                         CATString&       prefValue)
{
    this->fPrefLock.Wait();

    CATXMLObject* curObj = FindPref(prefSection,prefName); 

    if (curObj == 0)
    {
        this->fPrefLock.Release();
        return false;
    }

    prefValue = curObj->GetAttribute(L"Value");
    prefValue.Trim();

    this->fPrefLock.Release();
    return true;
}


//---------------------------------------------------------------------------
// Set the specified preference with whatever type
bool CATPrefs::SetPref(  const CATString& prefSection, 
                         const CATString& prefName, 
                         const CATString& prefValue)
{

    CATResult result = CAT_SUCCESS;

    // Trim incoming pref value.
    CATString val = prefValue;
    val.Trim();

    // Find the preference object if it exists...
    CATXMLObject* curObj = FindPref(prefSection,prefName); 

    // Check if we got it..
    if (curObj == 0)
    {
        // Need to add object to section
        CATXMLObject* parentObj = FindSection(prefSection);
        // Check if we need to add the section as well....
        if (parentObj == 0)
        {
            // Need to add the section too....
            parentObj = new CATXMLObject(prefSection);
            fRootNode->AddChild(parentObj);
        }

        curObj = new CATXMLObject(prefName);
        curObj->AddAttribute(L"Value",val);
        parentObj->AddChild(curObj);

        // All done, pref added.
        this->fPrefLock.Release();
        return true;
    }   

    // Pref already there - just set the value.
    if (CATSUCCEEDED(result = curObj->AddAttribute(L"Value",val)))
    {
        this->fPrefLock.Release();      
        return true;
    }

    // Error setting the value? bizarre... throw error.
    this->fPrefLock.Release();
    throw(result);

    return false;
}


//---------------------------------------------------------------------------
// Load() loads the preferences from the prefs file
//---------------------------------------------------------------------------
CATResult CATPrefs::Import(const CATString& path,
                           CATFileSystem*   fs)
{
    fPrefLock.Wait();
    // We're using pretty simple XML here, so no overriding is currently needed.
    // The root level elements are sections, the children are preferences.
    // Values for the preferences are kept in the "Value" attribute of the
    // preference entries.
    CATResult result = CAT_SUCCESS;

    CATXMLFactory factory;

    if (fs == 0)
    {
        if (gApp == 0)
        {
            this->fPrefLock.Release();
            return CATRESULT(CAT_ERR_PREFS_NO_FILESYSTEM);
        }

        fs = gApp->GetGlobalFileSystem();
    }
    
    if (fs == 0)
    {
        this->fPrefLock.Release();
        return CATRESULT(CAT_ERR_PREFS_NO_FILESYSTEM);
    }

    this->Clear();

    // If the file exists, then load the prefs.
    if (CATSUCCEEDED(result = fs->FileExists(path)))
    {      
        if (CATSUCCEEDED(result = CATXMLParser::Parse(   path, 
                                                         &factory, 
                                                         this->fRootNode)))
        {
            this->fPrefLock.Release();
            return result;
        }
        else
        {
            gApp->DisplayError(result);
        }
    }

    // An error occurred - clear the tree, set the root node
    this->Clear();

    this->fRootNode = new CATXMLObject(L"Preferences");

    // An error occurred - return it.
    this->fPrefLock.Release();
    return result;;
}


//---------------------------------------------------------------------------
// FindSection returns the XML object for the specified preference
//
// \return CATXMLObject* - preference object or 0 if not found.
//---------------------------------------------------------------------------
CATXMLObject* CATPrefs::FindPref(const CATString& prefSection, const CATString& prefName)
{
    CATXMLObject* curObj = 0;
    CATXMLObject* curSection = this->FindSection(prefSection);

    // Section doesn't exist.  Pref certainly doesn't.
    if (curSection == 0)
    {
        return curObj;
    }

    CATUInt32 i;
    CATUInt32 numChildren = curSection->GetNumChildren();
    for (i = 0; i < numChildren; i++)
    {
        curObj = curSection->GetChild(i);
        if (0 == wcscmp(curObj->GetType(),prefName))
        {
            return curObj;
        }
    }

    // Didn't find it.
    return 0;
}

//---------------------------------------------------------------------------
// FindSection returns the XML object for the preference section
//
// \return CATXMLObject* - section object or 0 if not found.
//---------------------------------------------------------------------------
CATXMLObject* CATPrefs::FindSection(const CATString& prefSection)
{
    CATXMLObject* curSection = 0;

    CATUInt32 i;
    CATUInt32 numChildren = fRootNode->GetNumChildren();
    for (i = 0; i < numChildren; i++)
    {
        curSection = fRootNode->GetChild(i);
        if (0 == wcscmp(curSection->GetType(),prefSection) )
        {
            return curSection;
        }
    }

    // Didn't find it.
    return 0;  
}

/// Save() save prefs to registry/file
CATResult CATPrefs::Save()
{
    
    return Export(this->fPrefFile);
}

CATResult CATPrefs::Load(CATFileSystem* fs)
{
    return Import(this->fPrefFile, fs);    
    //return CAT_SUCCESS;
}

/// Export() saves prefs to specified file location.
CATResult CATPrefs::Export(const CATString& path)
{
    fPrefLock.Wait();
    // We're using pretty simple XML here, so no overriding is currently needed.
    // The root level elements are sections, the children are preferences.
    // Values for the preferences are kept in the "Value" attribute of the
    // preference entries.
    CATResult result = CAT_SUCCESS;

    CATXMLFactory factory;

    result = CATXMLParser::Write(path,fRootNode);

    this->fPrefLock.Release();
    return result;;
}


