#include "CLA.h"

#include <algorithm> // std::min
#include <sstream>   // std::stringstream
#include <iomanip>   // std::left, std::setw

#include <assert.h>
#define error_if(x, err, msg, ...) do { if((x)) { mError = CLA::FormatString(msg, ## __VA_ARGS__); return err; } } while (false)

// Etc.
#include "CLAUtils.h"

namespace CLA {

	static CLA::String TypeToString(ValueType type) {
		switch (type) {
			case ValueType::Bool:
				return CLA_STRING("Bool");
			case ValueType::Double:
				return CLA_STRING("Double");
			case ValueType::Float:
				return CLA_STRING("Float");
			case ValueType::Int:
				return CLA_STRING("Int");
			case ValueType::String:
				return CLA_STRING("String");
			case ValueType::None:
				return CLA_STRING("None");
			default:
				return CLA_STRING("Invalid");
		}
	}

	ArgumentDescription::ArgumentDescription(EntryType entryType, const CLA::String &shortName, const CLA::String &longName, const CLA::String &description, ValueType valueType, int entryFlags) 
		: mEntryType(entryType)
		, mShortName(shortName)
		, mLongName(longName)
		, mDescription(description)
		, mValueType(valueType)
		, mEntryFlags(entryFlags) {

	}

	Parser::Parser(std::vector<ArgumentDescription> arguments)
		: mArgumentDescriptions(arguments)
		, mSwitchChars(CLA_STRING("-/")) {
	}

	Parser::Parser(ArgumentDescription *arguments, size_t argumentCount) {
		for (unsigned i = 0; i < argumentCount; ++i)
			mArgumentDescriptions.push_back(arguments[i]);

	}

	Parser::~Parser() {
		mArgumentDescriptions.clear();
	}

	CLA::Result Parser::parse(int argc, const char **argv) {
		error_if(argv == nullptr, Result::ErrorNullArgument, "Null arguments passed");
		error_if(argc <= 0, Result::ErrorArgCount, "Invalid number of argmuments passed");

		error_if(argv[0] == nullptr, Result::ErrorNullArgument, "Null executable name (argument 0)");
		mApplicationName = CLA::ExtractFilename(argv[0]);

		for (int i = 1; i < argc; ++i) {
			error_if(argv[i]== nullptr, Result::ErrorNullArgument, "Null argument passed");
			CLA::String currentArg(CLA::ToCLAString(argv[i]));

			bool hasSwitch = false;
			bool hasLongSwitch = false;
			for (auto &switchChar : mSwitchChars) {
				size_t switchPos = currentArg.find(switchChar);
				if (switchPos == 0) {
					hasSwitch = true;

					if (currentArg.find(switchChar, switchPos + 1) == 1)
						hasLongSwitch = true;
				}

			}

			if (hasLongSwitch || hasSwitch) {
				CLA::String parsedArg;

				if (hasLongSwitch) {
					parsedArg = currentArg.substr(2, CLA::String::npos);
				}
				else {
					parsedArg = currentArg.substr(1, CLA::String::npos);
				}

				CLA::String argName, argValue;

				auto eqPos = parsedArg.find_first_of('=');
				if (eqPos == CLA::String::npos) // Did not find '=' (ex: -o file.cpp, -editor)
				{
					argName = parsedArg;
				}
				else if (eqPos != CLA::String::npos) // Look for '=' (ex: -o=file.cpp)
				{
					argName = parsedArg.substr(0, eqPos);
					argValue = parsedArg.substr(eqPos + 1);
				}
				else // Assumed to be switch
				{
					argName = argValue = parsedArg;
				}

				if (argName.empty()) // Empty arg (ex: my.exe -)
				{
					mError = CLA_STRING("Invalid empty argument");
					return Result::ErrorUnkown;
				}

				// Search for description with the same name
				ArgumentDescription *argDesc = nullptr;
				for (auto &argDescItr : mArgumentDescriptions) {
					if ((argDescItr.mShortName == argName) || (argDescItr.mLongName == argName)) {
						argDesc = &argDescItr;
						break;
					}
				}

				if (!argDesc) // Invalid argument given
				{
					mError = CLA_STRING("Invalid argument flag '") + argName + CLA_STRING("'");
					return Result::ErrorUnkown;
				}

				EntryType argType = argDesc->mEntryType;

				if (argType == EntryType::Option) // Type logic
				{
					if (argValue.empty()) // Argument is in two parts (ex: -o file.cpp)
					{
						if (i >= argc - 1) // No value passed to arg
						{
							mError = CLA_STRING("Invalid argument value for flag '") + argName + CLA_STRING("'");
							return Result::ErrorUnkown;
						}

						argValue = CLA::ToCLAString(argv[++i]);
					}
				}

				if ((argDesc->mShortName == argName) && hasLongSwitch) // Long switch with short name
				{
					mError = CLA_STRING("Invalid argument flag '") + argName + CLA_STRING("'");
					return Result::ErrorUnkown;
				}

				if (argType == EntryType::Switch) // Switches
				{
					mSwitches.push_back(argName);
				}
				else // Options
				{
					// Store values
					if (!argDesc->mShortName.empty())
						mArgumentValues[argDesc->mShortName] = argValue;
					if (!argDesc->mLongName.empty())
						mArgumentValues[argDesc->mLongName] = argValue;
				}
			}
			else // Assumed to be parameter
			{
				mParameters.push_back(currentArg);
			}
		}

		// Check for all mandatory flags
		size_t paramCount = 0;
		for (auto &i : mArgumentDescriptions) {
			if (i.mEntryFlags & EntryFlags::Manditory) // Only check mandatory options
			{
				switch (i.mEntryType) {
					case EntryType::Switch:
						// Short and long not find
						error_if(!(findSwitch(i.mLongName) || findSwitch(i.mShortName)), Result::ErrorMissingArg, "Missing required switch '" CLA_FORMAT_STRING_SPECIFIER "'", i.mShortName.c_str());
						break;
					case EntryType::Parameter:
					{
						paramCount++;
						error_if(paramCount > mParameters.size(), Result::ErrorTooManyParams, "Too many parameters given");
						break;
					}
					default:
					{
						auto shortItr = mArgumentValues.find(i.mShortName);
						auto longItr = mArgumentValues.find(i.mLongName);

						// Short and long not find
						error_if((longItr == mArgumentValues.end()) && (shortItr == mArgumentValues.end()), Result::ErrorMissingArg, "Missing required option '" CLA_FORMAT_STRING_SPECIFIER "'", i.mShortName.c_str());
					}
					break;
				}
			}
		}

		return Result::OK;
	}

	bool Parser::find(const CLA::String &argument, CLA::String &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = *valueStr;
		return true;
	}

	bool Parser::find(const CLA::String &argument, bool &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}


		// Parse bool value from string
		CLA::String lowerValue(CLA::ToLower(*valueStr));

		if (lowerValue == CLA_STRING("true"))
			destination = true;
		else if (lowerValue == CLA_STRING("false"))
			destination = false;

		return true;
	}

	bool Parser::find(const CLA::String &argument, int &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = std::stoi(valueStr->c_str());
		return true;
	}

	bool Parser::find(const CLA::String &argument, float &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = static_cast<float>(std::stof(valueStr->c_str()));
		return true;
	}

	bool Parser::find(const CLA::String &argument, double &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = std::stof(valueStr->c_str());
		return true;
	}

	bool Parser::find(const CLA::String &argument, unsigned &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = static_cast<unsigned>(std::stoi(valueStr->c_str()));
		return true;
	}

	bool Parser::find(const CLA::String &argument, char &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = static_cast<char>(std::stoi(valueStr->c_str()));
		return true;
	}

	bool Parser::find(const CLA::String &argument, unsigned char &destination) const {
		auto valueStr = _getArgumentValue(argument);
		if (valueStr == nullptr) {
			return false;
		}

		destination = static_cast<unsigned char>(std::stoi(valueStr->c_str()));
		return true;
	}

	void Parser::setSwitchChars(const CLA::String &switchChars) {
		mSwitchChars = switchChars;
	}

	void Parser::setSwitchChars(char c) {
		mSwitchChars.resize(1);
		mSwitchChars[0] = c;
	}

	bool Parser::findSwitch(const CLA::String &argument) const {
		return (std::find(mSwitches.begin(), mSwitches.end(), _getLongArgName(argument)) != mSwitches.end()) || (std::find(mSwitches.begin(), mSwitches.end(), _getShortArgName(argument)) != mSwitches.end());
	}

	size_t Parser::getParamCount() const {
		return mParameters.size();
	}

	CLA::Result Parser::getParam(size_t paramIndex, CLA::String &destination) const {
		error_if(paramIndex >= mParameters.size(), Result::ErrorArgCount, "Invalid param. index");
		destination = mParameters[paramIndex];
		return CLA::Result::OK;
	}
	
	const CLA::String* Parser::_getArgumentValue(const CLA::String& argName) const {
		auto valueItr = mArgumentValues.find(_getLongArgName(argName));
		if (valueItr == mArgumentValues.end()) {
			valueItr = mArgumentValues.find(_getShortArgName(argName));
			if (valueItr == mArgumentValues.end())
				return nullptr;
		}

		return &valueItr->second;
	}

	CLA::String Parser::_getLongArgName(const CLA::String& argName) const {
		for (auto&& arg : mArgumentDescriptions) {
			if ((arg.mLongName == argName) || (arg.mShortName == argName)) {
				return arg.mLongName;
			}
		}

		return CLA::String();
	}

	CLA::String Parser::_getShortArgName(const CLA::String& argName) const {
		for (auto&& arg : mArgumentDescriptions) {
			if ((arg.mLongName == argName) || (arg.mShortName == argName)) {
				return arg.mShortName;
			}
		}

		return CLA::String();
	}

	void Parser::_generateUsageString() {
		std::vector<CLA::String> flagNames;
		std::vector<CLA::String> flagDescriptions;
		std::vector<CLA::String> argumentStrings;

		size_t longestName = 0;

		// Initial command line
		for (auto &i : mArgumentDescriptions) {
			bool optional = (i.mEntryFlags | EntryFlags::Optional) != 0;
			CLA::String argString = CLA_STRING(" ");

			// Prologue
			if (optional)
				argString += CLA_STRING("[");

			argString += mSwitchChars[0];
			argString += i.mShortName;

			// Generate usage name
			CLA::String flagName = mSwitchChars.substr(0, 1) + i.mShortName;
			if (!i.mLongName.empty()) {
				flagName += (CLA_STRING(", ") + mSwitchChars.substr(0, 1)) + mSwitchChars.substr(0, 1);
				flagName += i.mLongName;

				argString += (CLA_STRING("|") + mSwitchChars.substr(0, 1)) + mSwitchChars.substr(0, 1) + i.mLongName;
			}


			switch (i.mEntryType) {
				case EntryType::Option:
					break;
				case EntryType::Parameter:
					argString += CLA_STRING(" <") + TypeToString(i.mValueType) + CLA_STRING(">");
					flagName += CLA_STRING("=<") + TypeToString(i.mValueType) + CLA_STRING(">");
					break;
				case EntryType::Switch:
					break;
				case EntryType::UsageText:
					break;
				default:
					break;
			}

			// Epilogue
			if (optional) {
				argString += CLA_STRING("]");
			}

			flagDescriptions.push_back(i.mDescription);
			flagNames.push_back(flagName);
			argumentStrings.push_back(argString);

			size_t flagLength = flagName.length();
			if (flagLength > longestName)
				longestName = flagLength;
		}

		CLA::StringStream usageStream;
		usageStream << CLA_STRING("Usage: ") + mApplicationName;
		for (auto &i : argumentStrings)
			usageStream << i + CLA_STRING(" ");

		usageStream << std::endl;
		for (unsigned i = 0; i < flagNames.size(); ++i) {
			usageStream << std::left << std::setw(longestName) << flagNames[i] << CLA_STRING("\t") << flagDescriptions[i] << std::endl;
		}

		auto output = usageStream.str();
		mUsageString = output;
	}

	const CLA::String &Parser::getUsageString() {
		if (mUsageString.empty()) {
			_generateUsageString();
		}
		return mUsageString;
	}

}

