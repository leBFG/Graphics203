#pragma once
//#include <codecvt>
//#include <locale>
#include <string>
#include <algorithm>

namespace Skateboard
{
	//https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string

	__inline std::wstring ToWString(const std::string& str)
	{
		//Deprecated as of cxx17
	/*	using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converter;

		return converter.from_bytes(str);*/

		std::wstring wstr(str.length(), 0);
		std::transform(str.begin(), str.end(), wstr.begin(), [](char c) {
			return (wchar_t)c;
			});

		return wstr;
	}

	__inline std::string ToString(const std::wstring& wstr)
	{
		//Deprecated as of cxx17
		/*using convert_typeX = std::codecvt_utf8<wchar_t>;
		std::wstring_convert<convert_typeX, wchar_t> converter;

		return converter.to_bytes(wstr);*/

		std::string str(wstr.length(), 0);
		std::transform(wstr.begin(), wstr.end(), str.begin(), [](wchar_t c) {
			return (char)c;
			});

		return str;
	}
}
