#pragma once

// CLA
#include "CLAUtils.h"

// STL
#include <unordered_map>
#include <vector>

namespace CLA
{
  // Types of arguments
  enum class EntryType
  {
    Switch,
    Option,
    Parameter,
    UsageText,

    None
  };

  // Types of values for options and parameters
  enum class ValueType
  {
    Int,
    Float,
    Double,
    String,
    Bool,

    None
  };

  namespace EntryFlags
  {
    enum
    {
      None = 0,

      Optional =    1 << 1,
      Manditory =   1 << 2,
      Multiple =    1 << 3,
      Help =        1 << 4,
      Negatable =   1 << 5
    };
  }

  // Describes an argument to look for
  struct ArgumentDescription
  {
    EntryType     mEntryType;
    CLA::String   mShortName;
    CLA::String   mLongName;
    CLA::String   mDescription;
    ValueType     mValueType;
    int           mEntryFlags;

    ArgumentDescription(EntryType entryType, 
        const CLA::String &shortName, 
        const CLA::String &longName, 
        const CLA::String &description, 
        ValueType valueType, 
        int entryFlags);
  };

  enum class Result : int
  {
    OK,
    Help,

    ErrorUnkown,
    ErrorNullArgument,
    ErrorArgCount,
    ErrorMissingArg,
    ErrorTooManyParams,

    None
  };

  class Parser
  {
  public:
    Parser(ArgumentDescription *arguments, size_t argumentCount);
    Parser(std::vector<ArgumentDescription> arguments);
    ~Parser();

    Result parse(int argc, const char **argv);

    void setSwitchChars(const CLA::String &switchChars);
    void setSwitchChars(char c);

    bool findSwitch(const CLA::String &argument) const;
    bool find(const CLA::String &argument, CLA::String   &value) const;
    bool find(const CLA::String &argument, bool          &value) const;
    bool find(const CLA::String &argument, int           &value) const;
    bool find(const CLA::String &argument, unsigned      &value) const;
    bool find(const CLA::String &argument, float         &value) const;
    bool find(const CLA::String &argument, double        &value) const;
    bool find(const CLA::String &argument, char          &value) const;
    bool find(const CLA::String &argument, unsigned char &value) const;

    size_t getParamCount() const;
    CLA::Result getParam(size_t paramIndex, CLA::String &destination) const;
    const CLA::String &getError() const { return mError; };
    const CLA::String &getUsageString();

  private:
    const CLA::String* _getArgumentValue(const CLA::String& argName) const;
    CLA::String _getLongArgName(const CLA::String& argName) const;
    CLA::String _getShortArgName(const CLA::String& argName) const;

    // Generates usage based off of given values
    void _generateUsageString();

    // Arguments stored by their option
    typedef std::unordered_map<CLA::String, CLA::String> ArgumentMap;
    ArgumentMap mArgumentValues;

    // Switches found
    std::vector<CLA::String> mSwitches;

    // Parameters found
    std::vector<CLA::String> mParameters;

    std::vector<ArgumentDescription> mArgumentDescriptions;

    // Error string if any occur
    mutable CLA::String mError;

    // Characters that are used as switches
    CLA::String mSwitchChars;

    // Use string displayed if queried
    CLA::String mUsageString;
    
    // Name of application
    CLA::String mApplicationName;
  };

} // CL
