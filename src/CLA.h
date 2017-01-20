#pragma once

// STL
#include <unordered_map>
#include <string>

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
    EntryType     m_entryType;
    std::string   m_shortName;
    std::string   m_longName;
    std::string   m_description;
    ValueType     m_valueType;
    int           m_entryFlags;

    ArgumentDescription(EntryType entryType, const char *shortName, const char *longName, const char *description, ValueType valueType, int entryFlags);
	ArgumentDescription(EntryType entryType, const std::string& shortName, const std::string& longName, const std::string& description, ValueType valueType, int entryFlags);
  };

  enum class ParseResult : int
  {
    Valid,
    Invalid,
    Help,

    None
  };

  class Parser
  {
  public:
    Parser(ArgumentDescription *arguments, size_t argumentCount);
    Parser(std::vector<ArgumentDescription> arguments);

    ParseResult Parse(int argc, const char **argv);

    void SetSwitchChars(const std::string &switchChars);
    void SetSwitchChars(char c);

    bool FindSwitch(const std::string &argument) const;
    bool Find(const std::string &argument, std::string   &value) const;
    bool Find(const std::string &argument, bool          &value) const;
    bool Find(const std::string &argument, int           &value) const;
    bool Find(const std::string &argument, unsigned      &value) const;
    bool Find(const std::string &argument, float         &value) const;
    bool Find(const std::string &argument, double        &value) const;
    bool Find(const std::string &argument, char          &value) const;
    bool Find(const std::string &argument, unsigned char &value) const;

    size_t GetParamCount() const;
    const std::string &GetParam(size_t paramIndex) const;

    const std::string &GetError() const { return m_error; };

    const std::string &GetUsageString() const;

  private:
    // Generates usage based off of given values
    void generateUsageString();

    // Arguments stored by their option
    typedef std::unordered_map<std::string, std::string> ArgumentMap;
    ArgumentMap m_argumentValues;

    // Switches found
    std::vector<std::string> m_switches;

    // Parameters found
    std::vector<std::string> m_parameters;

    std::vector<ArgumentDescription> m_argumentDescriptions;

    // Error string if any occur
    std::string m_error;

    // Characters that are used as switches
    std::string m_switchChars;

    // Use string displayed if queried
    std::string m_usageString;
    
    // Name of application
    std::string m_applicationName;
  };

} // CL
