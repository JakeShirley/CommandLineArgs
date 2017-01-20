#pragma once

#include <string>
#include <sstream>

namespace CLA {
#ifdef CLA_UNICODE
	typedef std::wstring String;
	typedef wchar_t Character;
	typedef std::wstringstream StringStream;
#define CLA_FORMAT_STRING_SPECIFIER "%ls"
#define CLA_STRING(x) L ##x
#else
	typedef std::string String;
	typedef char Character;
	typedef std::stringstream StringStream;
#define CLA_FORMAT_STRING_SPECIFIER "%s"
#define CLA_STRING(x) x
#endif

	// Returns a copy of a lowered string
	String ToLower(const String &str);

	// Finds all instances of "find" and replaces it with "replaceWith"
	void ReplaceCharacters(String &str, Character find, Character replaceWith);

	// Extracts the filename and extension out of a path
	String ExtractFilename(const std::wstring &path);
	String ExtractFilename(const std::string &path);

	// Converts to the correct string type based on Unicode support being enabled
	String ToCLAString(const std::wstring &path);
	String ToCLAString(const std::string &path);

	std::wstring StringToWideString(const std::string &inputStr);
	std::string WideStringToString(const std::wstring &inputStr);

	String FormatString(const char *format, ...);
}