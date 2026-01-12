#if 1 // #if defined	(_MSC_VER)
#pragma message("Using ATLString CString class")
#else
// Provides MFC-like string support
#pragma message("Using portable CString class")
#include "windows.h" 
#include <malloc.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include "MigCstring.h"

#define DEBUGMSG(x)

/*
 *  default constructor
 *
 */
CString::CString()
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = "";
}

CString::CString(const CString& strSrc)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = strSrc;
}

CString::CString(TCHAR ch, int nRepeat)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = "";
	while (nRepeat--) *this += ch;
}

CString::CString(LPCSTR lpsz)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = lpsz;
}

CString::CString(const unsigned char* psz)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = psz;
}

CString::CString(int i)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = i;
}

CString::CString(long l)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = l;
}

CString::CString(unsigned long ul)
{
	m_pchData = NULL;
	m_iLen = 0;
	*this = ul;
}

CString::~CString()
{
	if (m_pchData) free(m_pchData);
}

void CString::LoadStringA(int uID)
{
	//TODO, something smart with the IDS number inside the rc file
#if 0
	WCHAR * pBuf = NULL;

	int len = LoadStringW( NULL, uID,reinterpret_cast< LPWSTR >(&pBuf),0);

	char * myNewStr = NULL;
	if (len)
	{
		wcstombs(myNewStr, pBuf, len);
		*this = myNewStr;
	}
#else
	return;
#endif
}

void CString::LoadStringA(char* myNewStr)
{
	*this = myNewStr;
}

int CString::GetLength()
{
	if (!m_pchData) return 0;
	return m_iLen;
}

BOOL CString::IsEmpty()
{
	return (!m_pchData || !m_iLen);
}

void CString::Empty()
{
	*this = "";
}

TCHAR CString::GetAt(int nIndex)
{
	if ((nIndex >= 0) && (nIndex < GetLength()))
		return m_pchData[nIndex];
	else
		return 0;
}

TCHAR CString::operator[](int nIndex)
{
	return GetAt(nIndex);
}

void CString::SetAt(int nIndex, TCHAR ch)
{
	if ((nIndex >= 0) && (nIndex < GetLength()))
		m_pchData[nIndex] = ch;
}

const CString& CString::operator=(const CString& strSrc)
{
	if (&strSrc == NULL) return *this;
	int nSrcLen = ((CString&) strSrc).GetLength();
	SetAllocSize(nSrcLen + 1);
	CopyMem(((CString&) strSrc).GetBuffer(), nSrcLen + 1);
	SetLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(TCHAR ch)
{
	int nSrcLen = sizeof(TCHAR);
	SetAllocSize(nSrcLen + 1);
	CopyMem(&ch, nSrcLen);
	SetLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(LPCSTR lpsz)
{
	if (!lpsz) return *this;
	int nSrcLen = strlen(lpsz);
	SetAllocSize(nSrcLen + 1);
	CopyMem((void *) lpsz, nSrcLen + 1);
	SetLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(const unsigned char* psz)
{
	if (!psz) return *this;
	int nSrcLen = strlen((char *) psz);
	SetAllocSize(nSrcLen + 1);
	CopyMem((void *) psz, nSrcLen + 1);
	SetLength(nSrcLen);
	return *this;
}

const CString& CString::operator=(int i)
{
	SetAllocSize(20);  // should be large enough for any number
	_itoa(i, m_pchData, 10);
	SetAllocSize(GetLength() + 1);
	SetLength(strlen(m_pchData));
	return *this;
}

const CString& CString::operator=(long l)
{
	SetAllocSize(20);  // should be large enough for any number
	_ltoa(l, m_pchData, 10);
	SetAllocSize(GetLength() + 1);
	SetLength(strlen(m_pchData));
	return *this;
}

const CString& CString::operator=(unsigned long ul)
{
	SetAllocSize(20);  // should be large enough for any number
	_ultoa(ul, m_pchData, 10);
	SetAllocSize(GetLength() + 1);
	SetLength(strlen(m_pchData));
	return *this;
}

CString::operator LPCTSTR()
{
	return this->GetBuffer();
}

const CString& CString::operator+=(const CString& string)
{
	if (&string == NULL) return *this;
	int nSrcLen = ((CString&) string).GetLength();
	SetAllocSize(GetLength() + nSrcLen + 1);
	AppendMem(((CString&) string).GetBuffer(), nSrcLen + 1);
	SetLength(GetLength() + nSrcLen);
	return *this;
}

const CString& CString::operator+=(TCHAR ch)
{
	int nSrcLen = sizeof(TCHAR);
	SetAllocSize(GetAllocSize() + 1);
	AppendMem(&ch, nSrcLen);
	SetLength(GetLength() + nSrcLen);
	return *this;
}

const CString& CString::operator+=(const unsigned char* psz)
{
	if (!psz) return *this;
	int nSrcLen = strlen((char *) psz);
	SetAllocSize(GetLength() + nSrcLen + 1);
	AppendMem((void *) psz, nSrcLen + 1);
	SetLength(GetLength() + nSrcLen);
	return *this;
}

CString __stdcall operator+(const CString& string1, const CString& string2)
{
	CString s;
	s = string1;
	s += string2;
	return s;
}

CString __stdcall operator+(const CString& string, TCHAR ch)
{
	CString s;
	s = string;
	s += ch;
	return s;
}

CString __stdcall operator+(TCHAR ch, const CString& string)
{
	CString s;
	s = ch;
	s += string;
	return s;
}

CString __stdcall operator+(const CString& string, LPCTSTR lpsz)
{
	CString s;
	s = string;
	s += lpsz;
	return s;
}

CString __stdcall operator+(LPCTSTR lpsz, const CString& string)
{
	CString s;
	s = lpsz;
	s += string;
	return s;
}

int CString::Compare(LPCTSTR lpsz)
{
	if (!lpsz) return 0;
	setlocale(LC_ALL, "");

	int iLenMe = GetLength();
	int iLenYou = strlen(lpsz);
	int iLenComp = (iLenMe < iLenYou) ? iLenMe : iLenYou;
	int iResult = memcmp(m_pchData, lpsz, iLenComp);
	if (iResult)
		return iResult;
	else
	{
		if (iLenMe == iLenYou) return 0;
		if (iLenMe < iLenYou) return -1;
		if (iLenMe > iLenYou) return 1;
	}
	return 0;  // never reached but to avoid c++ warning
}

int CString::CompareNoCase(LPCTSTR lpsz)
{
	if (!lpsz) return 0;
	setlocale(LC_ALL, "");

	int iLenMe = GetLength();
	int iLenYou = strlen(lpsz);
	int iLenComp = (iLenMe < iLenYou) ? iLenMe : iLenYou;
	int iResult = _memicmp(m_pchData, lpsz, iLenComp);
	if (iResult)
		return iResult;
	else
	{
		if (iLenMe == iLenYou) return 0;
		if (iLenMe < iLenYou) return -1;
		if (iLenMe > iLenYou) return 1;
	}
	return 0;  // never reached but to avoid c++ warning
}

BOOL CString::operator==(const char* psz)
{
	return !(this->Compare(psz));
}

BOOL CString::operator!=(const char* psz)
{
	return (this->Compare(psz));
}

CString CString::Mid(int nFirst, int nCount)
{
	// out-of-bounds requests return sensible things
	if (nFirst < 0)
		nFirst = 0;
	if (nCount < 0)
		nCount = 0;

	if (nFirst + nCount > GetLength())
		nCount = GetLength() - nFirst;
	if (nFirst > GetLength())
		nCount = 0;

	// optimize case of returning entire string
	if (nFirst == 0 && nFirst + nCount == GetLength())
		return *this;

	CString dest;
	dest = (GetBuffer() + nFirst);
	dest.SetLength(nCount);

	return dest;
}

CString CString::Mid(int nFirst)
{
	return Mid(nFirst, GetLength() - nFirst);
}

CString CString::Left(int nCount)
{
	return Mid(0, nCount);
}

CString CString::Right(int nCount)
{
	return Mid(GetLength() - nCount, nCount);
}

CString CString::Part(char cDelimiter, int nIndex)
{
	if (Find(cDelimiter) == -1) return *this;

	int iPos = 0;
	int iEnd;
	CString str;

	while (nIndex--)
	{
		iPos = Find(cDelimiter, iPos) + 1;
		if (!iPos) return str;
	}

	iEnd = Find(cDelimiter, iPos);
	if (iEnd == -1) iEnd = GetLength();

	str = Mid(iPos, iEnd - iPos);

	return str;
}

int CString::PartCount(char cDelimiter)
{
	if (IsEmpty()) return 0;
	CString str = *this;
	return str.Remove(cDelimiter) + 1;
}

int CString::PartBegin(char cDelimiter, int nIndex)
{
	if (Find(cDelimiter) == -1) return 0;
	if (IsEmpty()) return 0;
	if (nIndex == 0) return 0;

	int iPos = 0;
	while (nIndex--)
	{
		iPos = Find(cDelimiter, iPos) + 1;
		if (!iPos) return iPos;
	}

	return iPos;
}

void CString::MakeUpper()
{
	setlocale(LC_ALL, "");
	if (m_pchData) _strupr(m_pchData);
}

void CString::MakeLower()
{
	setlocale(LC_ALL, "");
	if (m_pchData) _strlwr(m_pchData);
}

void CString::MakeReverse()
{
	setlocale(LC_ALL, "");
	if (m_pchData) _strrev(m_pchData);
}

void CString::AnsiToOem()
{
	if (m_pchData) ::AnsiToOem(m_pchData, m_pchData);
}

void CString::OemToAnsi()
{
	if (m_pchData) ::OemToAnsi(m_pchData, m_pchData);
}

void CString::Escape()
{
	// TO-DO: escape string; see bo2ksrc/src/libc/strhandle.cpp
	DEBUGMSG("CString::Escape() is not yet implemented. String was not changed!");
}

void CString::UnEscape()
{
	// TO-DO: un-escape string; see bo2ksrc/src/libc/strhandle.cpp
	DEBUGMSG("CString::UnEscape() is not yet implemented. String was not changed!");
}

void CString::TrimRight()
{
	// check chars from the end on for space
	int iPos = GetLength() - 1;
	while (isspace(GetAt(iPos)) && (iPos >= 0))
		iPos--;
	iPos++;
	SetLength(iPos);
}

void CString::TrimLeft()
{
	// check chars from the beginning on for space
	int iPos = 0;
	int iLen = GetLength();
	while (isspace(GetAt(iPos)) && (iPos < iLen))
		iPos++;
	*this = Mid(iPos);
}

void CString::Trim()
{
	TrimLeft();
	TrimRight();
}

int CString::Replace(TCHAR chOld, TCHAR chNew)
{
	// check chars from the beginning on
	int iPos;
	int iLen = GetLength();
	int iCount = 0;
	for (iPos = 0; iPos < iLen; iPos++)
	{
		if (GetAt(iPos) == chOld)
		{
			SetAt(iPos, chNew);
			iCount++;
		}
	}
	return iCount;
}

int CString::Replace(LPCTSTR lpszOld, LPCTSTR lpszNew)
{
	if (!lpszOld) return -1;
	if (!lpszNew) return -1;

	// check string from the beginning on
	int iPos;
	int iLenOld = strlen(lpszOld);
	int iLen = GetLength() - iLenOld;
	int iCount = 0;
	for (iPos = 0; iPos < iLen; iPos++)
	{
		if (Mid(iPos, iLenOld) == lpszOld)
		{
			Delete(iPos, iLenOld);
			Insert(iPos, lpszNew);
			iCount++;
		}
	}
	return iCount;
}

int CString::Remove(TCHAR chRemove)
{
	// check chars from the beginning on
	int iPos;
	int iLen = GetLength();
	int iCount = 0;
	for (iPos = 0; iPos < iLen; iPos++)
	{
		if (GetAt(iPos) == chRemove)
		{
			Delete(iPos);
			iCount++;
		}
	}
	return iCount;
}

int CString::Insert(int nIndex, TCHAR ch)
{
	if (nIndex < 0) nIndex = 0;
	if (nIndex > GetLength()) nIndex = GetLength();

	SetAllocSize(GetAllocSize() + 1);

	// move anything right of new char to right
	int iPos;
	int iLen = GetLength();
	for (iPos = iLen; iPos >= nIndex; iPos--)
		SetAt(iPos + 1, GetAt(iPos));
	SetLength(++iLen);
	SetAt(nIndex, ch);

	return iLen;
}

int CString::Insert(int nIndex, LPCTSTR pstr)
{
	if (!pstr) return -1;
	if (nIndex < 0) nIndex = 0;
	if (nIndex > GetLength()) nIndex = GetLength();

	// TO-DO: do this like in Delete()
	// insert new string char by char
	int iPos;
	int iLen = strlen(pstr);
	for (iPos = 0; iPos < iLen; iPos++)
		Insert(nIndex + iPos, pstr[iPos]);

	return GetLength();
}

int CString::Delete(int nIndex, int nCount)
{
	if (nIndex < 0) nIndex = 0;
	if (nIndex > GetLength()) nIndex = GetLength();

	// move anything right of nIndex to left
	int iPos;
	int iLen = GetLength();
	for (iPos = nIndex; iPos + nCount <= iLen; iPos++)
		SetAt(iPos, GetAt(iPos + nCount));
	SetLength(iLen -= nCount);

	return iLen;
}

int CString::Find(TCHAR ch)
{
	return Find(ch, 0);
}

int CString::ReverseFind(TCHAR ch)
{
	int iPos;
	int iLen = GetLength();
	for (iPos = iLen; iPos >= 0; iPos--)
	{
		if (GetAt(iPos) == ch) return iPos;
	}
	return -1;
}

int CString::Find(TCHAR ch, int nStart)
{
	int iPos;
	int iLen = GetLength();
	for (iPos = nStart; iPos < iLen; iPos++)
	{
		if (GetAt(iPos) == ch) return iPos;
	}
	return -1;
}

int CString::Find(LPCTSTR lpszSub)
{
	return Find(lpszSub, 0);
}

int CString::Find(LPCTSTR lpszSub, int nStart)
{
	if (!lpszSub) return -1;

	int iPos;
	int iLen = GetLength();
	int iSLen = strlen(lpszSub);
	if (iSLen > iLen - nStart) return -1;  // is substring longer than my string, return -1

	for (iPos = nStart; iPos < iLen - iSLen; iPos++)
	{
		if (Mid(iPos, iSLen) == lpszSub) return iPos;
	}
	return -1;
}

void CString::Format(const char *pcFormat, ...)
{
	SetAllocSize(1024);

	va_list argList;
	va_start(argList, pcFormat);
	vsprintf(m_pchData, pcFormat, argList);
	va_end(argList);

	ReleaseBuffer();
}

LPTSTR CString::GetBuffer(int nMinBufLength)
{
	if (nMinBufLength != -1) SetAllocSize(nMinBufLength);

	return m_pchData;
}

void CString::ReleaseBuffer(int nNewLength)
{
	if (nNewLength != -1)
		SetLength(nNewLength);
	else
		SetLength(strlen(m_pchData));
	SetAllocSize(GetLength() + 1);
}



// private functions...

int CString::GetAllocSize()
{
	if (!m_pchData) return 0;
	return _msize(m_pchData);
}

void CString::SetAllocSize(int nSize)
{
	if (!m_pchData)
	{
		m_pchData = (char *) malloc(nSize);
		return;
	}

	if (GetAllocSize() != nSize)
	{
		m_pchData = (char *) realloc(m_pchData, nSize);
	}
}

void CString::CopyMem(void *pBuffer, int nLength)
{
	if (!pBuffer) return;
	if (!m_pchData) return;
	if (nLength <= 0) return;

	SetAllocSize(nLength + 1);

	memcpy(m_pchData, pBuffer, nLength);
}

void CString::AppendMem(void *pBuffer, int nLength)
{
	if (!pBuffer) return;
	if (!m_pchData) return;
	if (nLength <= 0) return;

	SetAllocSize(GetAllocSize() + nLength);

	memcpy(m_pchData + GetLength(), pBuffer, nLength);
}

void CString::SetLength(int nLength)
{
	if (!m_pchData) return;
	m_pchData[nLength] = 0;
	SetAllocSize(nLength + 1);
	m_iLen = nLength;
}

#endif
