//---------------------------------------------------------------------------
/// \file    CATPrefs.h
/// \brief   Preference storage class
/// \ingroup CATGUI
///
/// Copyright (c) 2003-2008 by Michael Ellison.
/// See COPYING.txt for license (MIT License).
///
// $Author: mike $
// $Date: 2011-05-30 17:06:23 -0500 (Mon, 30 May 2011) $
// $Revision: 3 $
// $NoKeywords: $
//
//---------------------------------------------------------------------------
#ifndef CATPrefs_H_
#define CATPrefs_H_

#include "CATInternal.h"
#include "CATString.h"
#include "CATXMLObject.h"
#include "CATMutex.h"

class CATFileSystem;

/// \class CATPrefs CATPrefs.h
/// \brief Preference storage class
/// \ingroup CATGUI
///
/// Changing this up a bit... making the interface more neutral for various
/// types of storage. Probably moving to registry for prefs on Windows for 
/// now.
class CATPrefs
{
public:
    /// CATPrefs constructor
    ///
    /// \param appName - name of prefs set (registry key/filepath/etc)
    /// If no none is given then load/save have no effect (runtime prefs)
    CATPrefs(const CATString& appName = "");

    virtual ~CATPrefs();

    // Note: the GetPref / Save pref simply get and save from and to memory.
    //       you must call Load/Save to persist settings.
    //       To revert, just call Load() without first calling Save().

    /// GetPref() retrieves the value of the requested preference.
    /// 
    /// \param prefSection - section name of preference (window name, etc.)
    /// \param prefName    - name of preference
    /// \param prefValue   - ref to value of preference (several overloaded types)
    /// \return bool - true if the preference existed, false otherwise.
    ///         If not found, prefValue remains unchanged.
    /// \sa SetPref()
    bool GetPref(const CATString& prefSection, const CATString& prefName, CATString&  prefValue);
    
    template<class T>
    bool GetPref(const CATString& prefSection, const CATString& prefName, T& prefValue)
    {
        CATString prefString;
        if (GetPref(prefSection, prefName, prefString))
        {
            // CATString performs conversion automatically.
            prefValue = (T)prefString;
            return true;
        }

        return false;
    }

    /// SetPref() sets the value of the specified preference.
    /// 
    /// \param prefSection - section name of preference (window name, etc.)
    /// \param prefName    - name of preference
    /// \param prefValue   - value of preference (several overloaded types)
    /// \return bool - true on success.
    /// \sa SetPref()
    bool SetPref(const CATString& prefSection, const CATString& prefName, const CATString&  prefValue);

    template<class T>
    bool SetPref(const CATString& prefSection, const CATString& prefName, const T prefValue)
    {
        CATString prefString(prefValue);
        return this->SetPref(prefSection,prefName,prefString);        
    }

    /// Save() save prefs to registry/file
    virtual CATResult   Save();

    /// Load() loads prefs from registry/file - WARNING: clears previous prefs!
    /// \param fs   FileSystem object, or NULL to use gApp to retrieve the file system.
    virtual CATResult   Load(CATFileSystem* fs = 0);

    /// Export() saves prefs to specified file location.
    virtual CATResult   Export(const CATString& path);
    
    /// Import() loads prefs from a specified file location
    /// \param path Path to file
    /// \param fs   FileSystem object, or NULL to use gApp to retrieve the file system.
    virtual CATResult   Import(const CATString& path,
                               CATFileSystem*   fs = 0);


    // Clear() resets all preferences (in memory).
    void        Clear();

    /// FindSection returns the XML object for the specified preference
    /// \return CATXMLObject* - preference object or 0 if not found.
    CATXMLObject*   FindPref(const CATString& prefSection, const CATString& prefName);

    /// FindSection returns the XML object for the preference section
    /// \return CATXMLObject* - section object or 0 if not found.
    CATXMLObject*   FindSection(const CATString& prefSection);

private:      
    CATString             fPrefFile;  // Preferences filename
    CATXMLObject*         fRootNode;  // Root XML object for sections.
    CATMutex              fPrefLock;  // Mutex for threadsafing.
};

#endif // CATPrefs_H_
