// ArchiverInfo.cpp

#include "StdAfx.h"

#include "ArchiverInfo.h"

#ifndef EXCLUDE_COM

#include "Common/StringConvert.h"
#include "Windows/FileFind.h"
#include "Windows/FileName.h"
#include "Windows/DLL.h"
#ifdef _WIN32
#include "Windows/Registry.h"
#endif
#include "Windows/PropVariant.h"
#include "../../Archive/IArchive.h"

using namespace NWindows;
using namespace NFile;

#endif

extern HINSTANCE g_hInstance;

#ifndef EXCLUDE_COM

static void SplitString(const UString &srcString, UStringVector &destStrings)
{
  destStrings.Clear();
  UString string;
  int len = srcString.Length();
  if (len == 0)
    return;
  for (int i = 0; i < len; i++)
  {
    wchar_t c = srcString[i];
    if (c == L' ')
    {
      if (!string.IsEmpty())
      {
        destStrings.Add(string);
        string.Empty();
      }
    }
    else
      string += c;
  }
  if (!string.IsEmpty())
    destStrings.Add(string);
}

typedef UInt32 (WINAPI * GetHandlerPropertyFunc)(
    PROPID propID, PROPVARIANT *value);

/*
UString GetCurrentModulePath()
{
  TCHAR fullPath[MAX_PATH + 1];
  ::GetModuleFileName(g_hInstance, fullPath, MAX_PATH);
  return fullPath;
}
*/

static UString GetModuleFolderPrefix()
{
  UString path;
  NDLL::MyGetModuleFileName(g_hInstance, path);
  int pos = path.ReverseFind(L'\\');
  return path.Left(pos + 1);
}

static wchar_t *kFormatFolderName = L"Formats";
static LPCTSTR kRegistryPath = TEXT("Software\\7-zip");
static LPCTSTR kProgramPathValue = TEXT("Path");

static UString GetBaseFolderPrefixFromRegistry()
{
  UString moduleFolderPrefix = GetModuleFolderPrefix();
  NFind::CFileInfoW fileInfo;
  if (NFind::FindFile(moduleFolderPrefix + kFormatFolderName, fileInfo))
    if (fileInfo.IsDirectory())
      return moduleFolderPrefix;
  CSysString pathSys;
  #ifdef _WIN32
  {
    NRegistry::CKey key;
    if(key.Open(HKEY_CURRENT_USER, kRegistryPath, KEY_READ) == ERROR_SUCCESS)
      if (key.QueryValue(kProgramPathValue, pathSys) == ERROR_SUCCESS)
      {
        UString path = GetUnicodeString(pathSys);
        NName::NormalizeDirPathPrefix(path);
        return path;
      }
  }
  {
    NRegistry::CKey key;
    if(key.Open(HKEY_LOCAL_MACHINE, kRegistryPath, KEY_READ) == ERROR_SUCCESS)
      if (key.QueryValue(kProgramPathValue, pathSys) == ERROR_SUCCESS)
      {
        UString path = GetUnicodeString(pathSys);
        NName::NormalizeDirPathPrefix(path);
        return path;
      }
  }
  #endif
  return moduleFolderPrefix;
}

typedef UInt32 (WINAPI *CreateObjectPointer)(
    const GUID *clsID, 
    const GUID *interfaceID, 
    void **outObject);

#endif

#ifndef _SFX
static void SetBuffer(CByteBuffer &bb, const Byte *data, int size)
{
  bb.SetCapacity(size);
  memmove((Byte *)bb, data, size);
}
#endif

void ReadArchiverInfoList(CObjectVector<CArchiverInfo> &archivers)
{
  archivers.Clear();
  
  #ifdef EXCLUDE_COM
  
  #ifdef FORMAT_7Z
  {
    CArchiverInfo item;
    item.UpdateEnabled = true;
    item.KeepName = false;
    item.Name = L"7z";
    item.Extensions.Add(CArchiverExtInfo(L"7z"));
    #ifndef _SFX
    const unsigned char kSig[] = {'7' , 'z', 0xBC, 0xAF, 0x27, 0x1C};
    SetBuffer(item.StartSignature, kSig, 6);
    #endif
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_BZIP2
  {
    CArchiverInfo item;
    item.UpdateEnabled = true;
    item.KeepName = true;
    item.Name = L"BZip2";
    item.Extensions.Add(CArchiverExtInfo(L"bz2"));
    item.Extensions.Add(CArchiverExtInfo(L"tbz2", L".tar"));
    #ifndef _SFX
    const unsigned char sig[] = {'B' , 'Z', 'h' };
    SetBuffer(item.StartSignature, sig, 3);
    #endif
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_GZIP
  {
    CArchiverInfo item;
    item.UpdateEnabled = true;
    item.KeepName = false;
    item.Name = L"GZip";
    item.Extensions.Add(CArchiverExtInfo(L"gz"));
    item.Extensions.Add(CArchiverExtInfo(L"tgz", L".tar"));
    #ifndef _SFX
    const unsigned char sig[] = { 0x1F, 0x8B };
    SetBuffer(item.StartSignature, sig, 2);
    #endif
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_SPLIT
  {
    CArchiverInfo item;
    item.UpdateEnabled = false;
    item.KeepName = true;
    item.Name = L"Split";
    item.Extensions.Add(CArchiverExtInfo(L"001"));
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_TAR
  {
    CArchiverInfo item;
    item.UpdateEnabled = true;
    item.KeepName = false;
    item.Name = L"Tar";
    item.Extensions.Add(CArchiverExtInfo(L"tar"));
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_ZIP
  {
    CArchiverInfo item;
    item.UpdateEnabled = true;
    item.KeepName = false;
    item.Name = L"Zip";
    item.Extensions.Add(CArchiverExtInfo(L"zip"));
    #ifndef _SFX
    const unsigned char sig[] = { 0x50, 0x4B, 0x03, 0x04 };
    SetBuffer(item.StartSignature, sig, 4);
    #endif
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_CPIO
  {
    CArchiverInfo item;
    item.UpdateEnabled = false;
    item.Name = L"Cpio";
    item.Extensions.Add(CArchiverExtInfo(L"cpio"));
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_RPM
  {
    CArchiverInfo item;
    item.UpdateEnabled = false;
    item.Name = L"Rpm";
    item.Extensions.Add(CArchiverExtInfo(L"rpm", L".cpio.gz"));
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_ARJ
  {
    CArchiverInfo item;
    item.UpdateEnabled = false;
    item.Name = L"Arj";
    item.Extensions.Add(CArchiverExtInfo(L"arj"));
    #ifndef _SFX
    const unsigned char sig[] = { 0x60, 0xEA };
    SetBuffer(item.StartSignature, sig, 2);
    #endif
    archivers.Add(item);
  }
  #endif

  #ifdef FORMAT_Z
  {
    CArchiverInfo item;
    item.UpdateEnabled = false;
    item.Name = L"Z";
    item.Extensions.Add(CArchiverExtInfo(L"Z"));
    #ifndef _SFX
    const unsigned char sig[] = { 0x1F, 0x9D };
    SetBuffer(item.StartSignature, sig, 2);
    #endif
    archivers.Add(item);
  }
  #endif
  
  #else

  UString folderPath = GetBaseFolderPrefixFromRegistry() + 
      kFormatFolderName + L"\\";
  NFind::CEnumeratorW enumerator(folderPath + L"*");
  NFind::CFileInfoW fileInfo;
  while (enumerator.Next(fileInfo))
  {
    if (fileInfo.IsDirectory())
      continue;
    UString filePath = folderPath + fileInfo.Name;
    {
      NDLL::CLibrary library;
      if (!library.LoadEx(filePath, LOAD_LIBRARY_AS_DATAFILE))
        continue;
    }

    NDLL::CLibrary library;
    if (!library.Load(filePath))
      continue;
    GetHandlerPropertyFunc getHandlerProperty = (GetHandlerPropertyFunc)
        library.GetProcAddress("GetHandlerProperty");
    if (getHandlerProperty == NULL)
      continue;

    CArchiverInfo item;
    item.FilePath = filePath;
    
    NWindows::NCOM::CPropVariant prop;
    if (getHandlerProperty(NArchive::kName, &prop) != S_OK)
      continue;
    if (prop.vt != VT_BSTR)
      continue;
    item.Name = prop.bstrVal;
    prop.Clear();

    if (getHandlerProperty(NArchive::kClassID, &prop) != S_OK)
      continue;
    if (prop.vt != VT_BSTR)
      continue;
    item.ClassID = *(const GUID *)prop.bstrVal;
    prop.Clear();

    if (getHandlerProperty(NArchive::kExtension, &prop) != S_OK)
      continue;
    if (prop.vt != VT_BSTR)
      continue;

    UString ext  = prop.bstrVal;
    // item.Extension = prop.bstrVal;
    prop.Clear();

    UString addExt;

    if (getHandlerProperty(NArchive::kAddExtension, &prop) != S_OK)
      continue;
    if (prop.vt == VT_BSTR)
    {
      addExt = prop.bstrVal;
    }
    else if (prop.vt != VT_EMPTY)
      continue;
    prop.Clear();

    UStringVector exts, addExts;
    SplitString(ext, exts);
    SplitString(addExt, addExts);

    prop.Clear();
    for (int i = 0; i < exts.Size(); i++)
    {
      CArchiverExtInfo extInfo;
      extInfo.Extension = exts[i];
      if (addExts.Size() > 0)
        extInfo.AddExtension = addExts[i];
      if (extInfo.AddExtension == L"*")
        extInfo.AddExtension.Empty();
      item.Extensions.Add(extInfo);
    }

    if (getHandlerProperty(NArchive::kUpdate, &prop) == S_OK)
      if (prop.vt == VT_BOOL)
        item.UpdateEnabled = VARIANT_BOOLToBool(prop.boolVal);
    prop.Clear();

    if (item.UpdateEnabled)
    {
      if (getHandlerProperty(NArchive::kKeepName, &prop) == S_OK)
        if (prop.vt == VT_BOOL)
          item.KeepName = VARIANT_BOOLToBool(prop.boolVal);
      prop.Clear();
    }

    if (getHandlerProperty(NArchive::kStartSignature, &prop) == S_OK)
    {
      if (prop.vt == VT_BSTR)
      {
        UINT len = ::SysStringByteLen(prop.bstrVal);
        item.StartSignature.SetCapacity(len);
        memmove(item.StartSignature, prop.bstrVal, len);
      }
    }
    prop.Clear();

    archivers.Add(item);
  }

  #endif
}


