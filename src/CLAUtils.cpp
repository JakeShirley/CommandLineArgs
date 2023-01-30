#include "CLAUtils.h"

// STL
#include <algorithm>  // std::transform
#include <functional> // stdd::not1, std::ptr_fun
#include <cctype>     // std::isspace
#include <locale>     // std::wstring_convert, std::tolower
#include <codecvt>    // std::codecvt_utf8
#include <cstdio>     // std::vsnprintf
#include <cstdarg>    // va_start, va_end

namespace CLA {
	void ReplaceCharacters(String &str, Character find, Character replaceWith) {
		for (size_t i = 0; i < str.length(); ++i)
			if (str[i] == find)
				str[i] = replaceWith;
	}

	String ExtractFilename(const std::wstring &path) {
		String result = ToCLAString(path);

		ReplaceCharacters(result, '/', '\\');

		size_t filenameStart = result.find_last_of('\\');
		filenameStart = (filenameStart == String::npos) ? 0 : filenameStart + 1;
		result.erase(0, filenameStart);

		return result;
	}

	String ExtractFilename(const std::string &path) {
		return ExtractFilename(StringToWideString(path));
	}

	String ToCLAString(const std::wstring &path) {
#ifdef CLA_UNICODE
		return path;
#else
		return WideStringToString(path);
#endif
	}

	String ToCLAString(const std::string &path) {
#ifdef CLA_UNICODE
		return StringToWideString(path);
#else
		return path;
#endif
	}

	std::wstring StringToWideString(const std::string &inputStr) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		return converter.from_bytes(inputStr);
	}

	std::string WideStringToString(const std::wstring &inputStr) {
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;

		return converter.to_bytes(inputStr);
	}

	CLA::String FormatString(const char *format, ...) {
		CLA::String returnValue;

		va_list args;
		va_start(args, format);

		char strBuffer[1024];
		std::vsnprintf(strBuffer, sizeof(strBuffer), format, args);

		va_end(args);

		returnValue = ToCLAString(strBuffer);
		return std::move(returnValue);
	}

	String ToLower(const String &str) {
		String retValue(str);
		std::transform(retValue.begin(), retValue.end(), retValue.begin(), [](char c) {
			return std::tolower(c, std::locale());
		});
		return retValue;
	}

}