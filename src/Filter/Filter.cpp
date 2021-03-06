/*MIT License

Copyright (c) 2016 Archer Xu

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/


#include "stdafx.h"
#include <functional>
#include <algorithm>

#ifndef LIB_WINDOWS
#include <ctype.h>
int _wcsupr_s(wchar_t* wzIn)
{
	if (wzIn == NULL)
	{
		return 0;
	}
	wchar_t *pwzCur = wzIn;

	while ((*pwzCur) != L'\0')
	{
		*pwzCur = towupper((*pwzCur));
		pwzCur++;
	}
	
	return 0;
}

int _strupr_s(char* szIn)
{
	if (szIn == NULL)
	{
		return 0;
	}
	char *pszCur = szIn;

	while ((*pszCur) != '\0')
	{
		*pszCur = toupper((*pszCur));
		pszCur++;
	}

	return 0;
}

int _wcsupr_s(wchar_t* wzIn, int nSize)
{
	if (wzIn == NULL)
	{
		return 0;
	}
	wchar_t *pwzCur = wzIn;
	int nCurSize = 0;
	while ((*pwzCur) != L'\0' && nCurSize < nSize)
	{
		*pwzCur = towupper((*pwzCur));
		pwzCur++;
		nCurSize++;
	}

	return 0;
}

int _strupr_s(char* szIn, int nSize)
{
	if (szIn == NULL)
	{
		return 0;
	}
	char *pszCur = szIn;
	int nCurSize = 0;
	while ((*pszCur) != '\0' && nCurSize < nSize)
	{
		*pszCur = toupper((*pszCur));
		pszCur++;
		nCurSize++;
	}

	return 0;
}
#endif // LIB_WINDOWS

const unsigned char kFirstBitMask = 128; // 1000000
const unsigned char kSecondBitMask = 64; // 0100000
const unsigned char kThirdBitMask = 32; // 0010000
const unsigned char kFourthBitMask = 16; // 0001000
const unsigned char kFifthBitMask = 8; // 0000100
const unsigned char kSixthBitMask = 4; // 0000010
int utf8_char_lenth(char firstByte)
{
	int offset = 1;

	if (firstByte & kFirstBitMask) // This means the first byte has a value greater than 127, and so is beyond the ASCII range.
	{
		if (firstByte & kThirdBitMask) // This means that the first byte has a value greater than 224, and so it must be at least a three-octet code point.
		{
			if (firstByte & kFourthBitMask) // This means that the first byte has a value greater than 240, and so it must be a four-octet code point.
			{
				if (firstByte & kFifthBitMask)
				{
					if (firstByte & kSixthBitMask)
					{
						offset = 6;
					}
					else
					{
						offset = 5;
					}
				}
				else
				{
					offset = 4;
				}
			}
			else
			{
				offset = 3;
			}
		}
		else
		{
			offset = 2;
		}
	}
	return offset;
}

namespace YTSvrLib
{

	void CKeywordFilterW::AddKeyWord(wchar_t* pwzKeyWord)
	{
		if (pwzKeyWord && *pwzKeyWord != L'\0')
		{
			wchar_t wzKeys[33] = { 0 };
			wcsncpy_s(wzKeys, pwzKeyWord, 32);
			wzKeys[32] = L'\0';

			_wcsupr_s(wzKeys);
			m_vctKeyword.push_back(std::wstring(wzKeys));
		}
	}
	bool WStrGreater(std::wstring& w1, std::wstring& w2)
	{
		return w1 > w2;
	}

	void CKeywordFilterW::Preprocess()
	{
		std::sort(m_vctKeyword.begin(), m_vctKeyword.end(), std::greater<std::wstring>());

		std::vector< std::wstring >::iterator it = m_vctKeyword.begin();

		while (it != m_vctKeyword.end())
		{
			wchar_t Character = (*it).c_str()[0];

			std::vector< std::wstring >::iterator itLower = it;
			std::vector< std::wstring >::iterator itUpper = it++;
			while (itUpper != m_vctKeyword.end() && (*itUpper).c_str()[0] == Character)
				itUpper++;
			ITPAIR aPair = std::make_pair(itLower, itUpper);
			m_mpKeyword[Character] = aPair;
			it = itUpper;
		}
		LOG("KeywordFilter KeyWord=%d", m_vctKeyword.size());
	}
	BOOL CKeywordFilterW::HasKeyWord(const wchar_t* pstrContent, long nTextLen, wchar_t* pstrOutKey, long nOutMaxLength)
	{
		wchar_t wzTemp[4096];
		long nLen = 0;
		for (long i = 0; i < nTextLen&&nLen < 8191; i++)
		if (pstrContent[i] != L' ')
		{
			wzTemp[nLen] = pstrContent[i];
			nLen++;
		}
		wzTemp[nLen] = L'\0';
		_wcsupr_s(wzTemp, nLen + 1);
		wchar_t* pIndex = wzTemp;
		wchar_t aCharacter = *pIndex;

		long nPattLen = 0;
		long nLenLeft = nLen;//nTextLen;

		while (*pIndex)
		{
			aCharacter = *pIndex;
			FilterIndex::iterator it = m_mpKeyword.find(aCharacter);
			if (it != m_mpKeyword.end())
			{
				ITPAIR& aPair = it->second;
				for (std::vector< std::wstring >::iterator itSub = aPair.first; itSub != aPair.second; ++itSub)
				{
					nPattLen = (long) (*itSub).length();

					if (nPattLen > nLenLeft)
					{
						continue;
					}

					if (0 == wcsncmp(pIndex, (*itSub).c_str(), nPattLen))
					{
						if (pstrOutKey && nOutMaxLength > 0)
						{
#ifdef LIB_WINDOWS
							wcsncpy_s(pstrOutKey, nOutMaxLength, (*itSub).c_str(), nOutMaxLength);
#else
							wcsncpy_s(pstrOutKey, (*itSub).c_str(), nOutMaxLength);
#endif // LIB_WINDOWS
						}
						return TRUE;
					}
				}
				++pIndex;
				--nLenLeft;
			}
			else
			{
				while (aCharacter == *++pIndex)
				{
					NULL;
				}
				if (*pIndex == 0)
				{
					return FALSE;
				}
			}
		}
		return FALSE;
	}
	void CKeywordFilterW::DoFilter(wchar_t* pstrContent, long nTextLen)
	{
		wchar_t wzStrNoSpace[2048];
		int	nPosInSrcString[2048];
		long nLen = 0;
		for (long i = 0; i < nTextLen&&nLen < 1023; i++)
		if (pstrContent[i] != L' ')
		{
			wzStrNoSpace[nLen] = pstrContent[i];
			nPosInSrcString[nLen] = i;
			nLen++;
		}
		wzStrNoSpace[nLen] = L'\0';
		_wcsupr_s(wzStrNoSpace, nLen + 1);

		//wchar_t* pIndex = pstrContent;
		wchar_t aCharacter = 0;

		long nPattLen = 0;
		long nLenLeft = nTextLen;
		bool bFind = false;

		long l = 0;
		while (l < nLen)
		{
			if (aCharacter == wzStrNoSpace[l])
			{
				l++;
				nLenLeft--;
				continue;
			}
			aCharacter = wzStrNoSpace[l];
			bFind = false;
			FilterIndex::iterator it = m_mpKeyword.find(aCharacter);
			if (it != m_mpKeyword.end())
			{
				ITPAIR& aPair = it->second;
				for (std::vector< std::wstring >::iterator itSub = aPair.first; itSub != aPair.second; ++itSub)
				{
					nPattLen = (long) (*itSub).length();
					if (nPattLen > nLenLeft)
						continue;

					if (0 == wcsncmp((LPCWSTR) (wzStrNoSpace + l), (*itSub).c_str(), nPattLen))
					{
						bFind = true;
						nLenLeft -= nPattLen;
						if (nPattLen == 1)
						{
							for (long i = 0; i < nPattLen; ++i)
							{
								if (nPosInSrcString[l] < nTextLen)
									pstrContent[nPosInSrcString[l]] = L'*';
								l++;
							}
						}
						else if (nPattLen == 2)
						{
							for (long i = 0; i < nPattLen; ++i)
							{
								if (nPosInSrcString[l] < nTextLen && i != 0)
								{
									pstrContent[nPosInSrcString[l]] = L'*';
								}
								l++;
							}
						}
						else if (nPattLen >= 3)
						{
							for (long i = 0; i < nPattLen; ++i)
							{
								if (nPosInSrcString[l] < nTextLen && i != 0 && i != (nPattLen - 1))
								{
									pstrContent[nPosInSrcString[l]] = L'*';
								}
								l++;
							}
						}

						break;
					}
				}
			}
			if (!bFind)
			{
				l++;
				--nLenLeft;
			}
		}//while( l < nLen )
	}

	void CKeywordFilterA::AddKeyWord(char* pszKeyWord)
	{
		if (pszKeyWord && (*pszKeyWord) != '\0')
		{
			char szKeys[33] = { 0 };
			strncpy_s(szKeys, pszKeyWord, 32);
			szKeys[32] = '\0';

			_strupr_s(szKeys);
			m_vctKeyword.push_back(std::string(szKeys));
		}
	}

	void CKeywordFilterA::Preprocess()
	{
		std::sort(m_vctKeyword.begin(), m_vctKeyword.end(), std::greater<std::string>());

		std::vector< std::string >::iterator it = m_vctKeyword.begin();

		while (it != m_vctKeyword.end())
		{
			char Character = (*it).c_str()[0];

			std::vector< std::string >::iterator itLower = it;
			std::vector< std::string >::iterator itUpper = it++;
			while (itUpper != m_vctKeyword.end() && (*itUpper).c_str()[0] == Character)
				itUpper++;
			ITPAIR aPair = std::make_pair(itLower, itUpper);
			m_mpKeyword[Character] = aPair;
			it = itUpper;
		}
		LOG("KeywordFilter KeyWord=%d", m_vctKeyword.size());
	}

	void CKeywordFilterA::DoFilter(char* pstrContent, long nTextLen)
	{
		char szStrNoSpace[2048] = { 0 };
		int	nPosInSrcString[2048] = { 0 };
		long nLen = 0;
		for (long i = 0; i < nTextLen&&nLen < 2047; i++)
		{
			if (pstrContent[i] != ' ')
			{
				szStrNoSpace[nLen] = pstrContent[i];
				nPosInSrcString[nLen] = i;
				nLen++;
			}
		}

		szStrNoSpace[nLen] = '\0';
		_strupr_s(szStrNoSpace, nLen + 1);

		//wchar_t* pIndex = pstrContent;
		char aCharacter = 0;

		long nPattLen = 0;
		long nLenLeft = nTextLen;
		bool bFind = false;

		long l = 0;
		while (l < nLen)
		{
			if (aCharacter == szStrNoSpace[l])
			{
				l++;
				nLenLeft--;
				continue;
			}
			aCharacter = szStrNoSpace[l];
			bFind = false;
			FilterIndex::iterator it = m_mpKeyword.find(aCharacter);
			if (it != m_mpKeyword.end())
			{
				ITPAIR& aPair = it->second;
				for (std::vector< std::string >::iterator itSub = aPair.first; itSub != aPair.second; ++itSub)
				{
					nPattLen = (long) (*itSub).length();
					if (nPattLen > nLenLeft)
						continue;

					if (0 == strncmp((LPCSTR) (szStrNoSpace + l), (*itSub).c_str(), nPattLen))
					{
						bFind = true;
						nLenLeft -= nPattLen;

						int nMaxCharlen = 0;
						for (size_t i = 0; i < (*itSub).size();++i)
						{
							int nCharlen = utf8_char_lenth((*itSub)[i]);
							if (nCharlen > nMaxCharlen)
							{
								nMaxCharlen = nCharlen;
							}
						}

						if (nMaxCharlen > 1)
						{// 这说明这个字符串包含了不属于ascii的字符,不能部分替换会导致字符串错误
							for (long i = 0; i < nPattLen; ++i)
							{
								if (nPosInSrcString[l] < nTextLen)
									pstrContent[nPosInSrcString[l]] = '*';
								l++;
							}
						}
						else
						{
							if (nPattLen == 1)
							{
								for (long i = 0; i < nPattLen; ++i)
								{
									if (nPosInSrcString[l] < nTextLen)
										pstrContent[nPosInSrcString[l]] = '*';
									l++;
								}
							}
							else if (nPattLen == 2)
							{
								for (long i = 0; i < nPattLen; ++i)
								{
									if (nPosInSrcString[l] < nTextLen && i != 0)
									{
										pstrContent[nPosInSrcString[l]] = '*';
									}
									l++;
								}
							}
							else if (nPattLen >= 3)
							{
								for (long i = 0; i < nPattLen; ++i)
								{
									if (nPosInSrcString[l] < nTextLen && i != 0 && i != (nPattLen - 1))
									{
										pstrContent[nPosInSrcString[l]] = '*';
									}
									l++;
								}
							}
						}

						break;
					}
				}
			}
			if (!bFind)
			{
				l++;
				--nLenLeft;
			}
		}//while( l < nLen )
	}

	BOOL CKeywordFilterA::HasKeyWord(const char* pstrContent, long nTextLen, char* pstrOutKey, long nOutMaxLength)
	{
		char szTemp[4096] = { 0 };
		long nLen = 0;
		for (long i = 0; i < nTextLen&&nLen < 8191; i++)
		{
			if (pstrContent[i] != ' ')
			{
				szTemp[nLen] = pstrContent[i];
				nLen++;
			}
		}
		szTemp[nLen] = '\0';
		_strupr_s(szTemp, nLen + 1);
		char* pIndex = szTemp;
		char aCharacter = *pIndex;

		long nPattLen = 0;
		long nLenLeft = nLen;//nTextLen;

		while (*pIndex)
		{
			aCharacter = *pIndex;
			FilterIndex::iterator it = m_mpKeyword.find(aCharacter);
			if (it != m_mpKeyword.end())
			{
				ITPAIR& aPair = it->second;
				for (std::vector< std::string >::iterator itSub = aPair.first; itSub != aPair.second; ++itSub)
				{
					nPattLen = (long) (*itSub).length();

					if (nPattLen > nLenLeft)
					{
						continue;
					}

					if (0 == strncmp(pIndex, (*itSub).c_str(), nPattLen))
					{
						if (pstrOutKey && nOutMaxLength > 0)
						{
							__strncpy_s(pstrOutKey, (*itSub).c_str(), nOutMaxLength);
						}
						return TRUE;
					}
				}
				++pIndex;
				--nLenLeft;
			}
			else
			{
				while (aCharacter == *++pIndex)
				{
					NULL;
				}
				if (*pIndex == 0)
				{
					return FALSE;
				}
			}
		}
		return FALSE;
	}
}
