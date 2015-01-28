#include "CLA.h"

#include <algorithm> // std::min
#include <sstream>   // std::stringstream
#include <iomanip>   // std:;left, std::setw

#include <assert.h>
#define Assert(x) assert(x)

// Etc.
#include "CLAUtils.h"

namespace CLA
{

  static std::string TypeToString(ValueType type)
  {
    switch(type)
    {
    case ValueType::Bool:
      return "Bool";
    case ValueType::Double:
      return "Double";
    case ValueType::Float:
      return "Float";
    case ValueType::Int:
      return "Int";
    case ValueType::String:
      return "String";
    case ValueType::None:
      return "None";
    default:
      return "Invalid";
    }
  }

  ArgumentDescription::ArgumentDescription(EntryType entryType, const char *shortName, const char *longName, const char *description, ValueType valueType, int entryFlags)
    : m_entryType(entryType),
      m_shortName(shortName ? shortName : ""),
      m_longName(longName ? longName : ""),
      m_description(description ? description : ""),
      m_valueType(valueType),
      m_entryFlags(entryFlags)
  {

  }

  Parser::Parser(std::vector<ArgumentDescription> arguments)
    : m_argumentDescriptions(arguments),
      m_switchChars("-/")
  {
    generateUsageString();
  }

  Parser::Parser(ArgumentDescription *arguments, size_t argumentCount)
  {
    Assert(arguments);
    Assert(argumentCount > 0);

    for(unsigned i = 0; i < argumentCount; ++i)
      m_argumentDescriptions.push_back(arguments[i]);

    generateUsageString();
  }

  CLA::ParseResult Parser::Parse(int argc, const char **argv)
{
    Assert(argv);
    Assert(argc > 0);

    m_applicationName = CLA::ExtractFilename(argv[0]);

    for(int i = 1; i < argc; ++i)
    {
      Assert(argv[i]);
      std::string currentArg(argv[i]);

      bool hasSwitch = false;
      bool hasLongSwitch = false;
      for(auto &i : m_switchChars)
      {
        int switchPos = currentArg.find(i);
        if(switchPos == 0)
        {
          hasSwitch = true;

          if(currentArg.find(i, switchPos + 1) == 1)
            hasLongSwitch = true;
        }
          
      }
      
      if(hasLongSwitch || hasSwitch)
      {
        std::string parsedArg;

        if(hasLongSwitch)
        {
          parsedArg = currentArg.substr(2, std::string::npos);
        }
        else
        {
          parsedArg = currentArg.substr(1, std::string::npos);
        }

        std::string argName, argValue;

        auto eqPos = parsedArg.find_first_of('=');
        if(eqPos == std::string::npos) // Did not find '=' (ex: -o file.cpp, -editor)
        {
          argName = parsedArg;
        }
        else if(eqPos != std::string::npos) // Look for '=' (ex: -o=file.cpp)
        {
          argName = parsedArg.substr(0, eqPos);
          argValue = parsedArg.substr(eqPos + 1);
        }
        else // Assumed to be switch
        {
          argName = argValue = parsedArg;
        }

        if(argName.empty()) // Empty arg (ex: my.exe -)
        {
          m_error = "Invalid empty argument";
          return ParseResult::Invalid;
        }

        // Search for description with the same name
        ArgumentDescription *argDesc = nullptr;
        for(auto &i : m_argumentDescriptions)
        {
          if((i.m_shortName == argName) || (i.m_longName == argName))
          {
            argDesc = &i;
            break;
          }
        }

        if(!argDesc) // Invalid argument given
        {
          m_error = "Invalid argument flag '" + argName + "'";
          return ParseResult::Invalid;
        }

        EntryType argType = argDesc->m_entryType;

        if(argType == EntryType::Option) // Type logic
        {
          if(argValue.empty()) // Argument is in two parts (ex: -o file.cpp)
          {
            if(i >= argc - 1) // No value passed to arg
            {
              m_error = "Invalid argument value for flag '" + argName + "'";
              return ParseResult::Invalid;
            }

            argValue = argv[++i];
          }
        }

        if((argDesc->m_shortName == argName) && hasLongSwitch) // Long switch with short name
        {
          m_error = "Invalid argument flag '" + argName + "'";
          return ParseResult::Invalid;
        }
        
        if(argType == EntryType::Switch) // Switches
        {
          m_switches.push_back(argName);
        }
        else // Options
        {
          // Store values
          if(!argDesc->m_shortName.empty())
            m_argumentValues[argDesc->m_shortName] = argValue;
          if(!argDesc->m_longName.empty())
            m_argumentValues[argDesc->m_longName] = argValue;
        }
      }
      else // Assumed to be parameter
      {
        m_parameters.push_back(currentArg);
      }
    }

    // Check for all mandatory flags
    for(auto &i : m_argumentDescriptions)
    {
      if(i.m_entryFlags & EntryFlags::Manditory) // Only check mandatory options
      {
        switch(i.m_entryType)
        {
        case EntryType::Switch:
          if(!(FoundSwitch(i.m_longName) || FoundSwitch(i.m_shortName))) // Short and long not found
          {
            m_error = "Missing required switch '" + i.m_shortName + "'";
            return ParseResult::Invalid;
          }
          break;
        default:
        {
          auto shortItr = m_argumentValues.find(i.m_shortName);
          auto longItr = m_argumentValues.find(i.m_longName);

          if((longItr == m_argumentValues.end()) && (shortItr == m_argumentValues.end())) // Short and long not found
          {
            m_error = "Missing required option '" + i.m_shortName + "'";
            return ParseResult::Invalid;
          }
        }
        break;
        }
      }
    }

    return ParseResult::Valid;
  }

  bool Parser::Found(const std::string &argument, std::string &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = valueItr->second;
    return true;
  }

  bool Parser::Found(const std::string &argument, bool &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;


    // Parse bool value from string
    std::string lowerValue(CLA::ToLower(valueItr->second));

    if(lowerValue == "true")
      destination = true;
    else if(lowerValue == "false")
      destination = false;

    return true;
  }

  bool Parser::Found(const std::string &argument, int &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = std::atoi(valueItr->second.c_str());
    return true;
  }

  bool Parser::Found(const std::string &argument, float &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = static_cast<float>(std::atof(valueItr->second.c_str()));
    return true;
  }

  bool Parser::Found(const std::string &argument, double &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = std::atof(valueItr->second.c_str());
    return true;
  }

  bool Parser::Found(const std::string &argument, unsigned &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = static_cast<unsigned>(std::atoi(valueItr->second.c_str()));
    return true;
  }

  bool Parser::Found(const std::string &argument, char &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = static_cast<char>(std::atoi(valueItr->second.c_str()));
    return true;
  }

  bool Parser::Found(const std::string &argument, unsigned char &destination) const
  {
    auto valueItr = m_argumentValues.find(argument);
    if(valueItr == m_argumentValues.end())
      return false;

    destination = static_cast<unsigned char>(std::atoi(valueItr->second.c_str()));
    return true;
  }

  void Parser::SetSwitchChars(const std::string &switchChars)
  {
    m_switchChars = switchChars;
  }

  void Parser::SetSwitchChars(char c)
  {
    m_switchChars.resize(1);
    m_switchChars[0] = c;
  }

  bool Parser::FoundSwitch(const std::string &argument) const
  {
    return std::find(m_switches.begin(), m_switches.end(), argument) != m_switches.end();
  }

  size_t Parser::GetParamCount() const
  {
    return m_parameters.size();
  }

  const std::string &Parser::GetParam(size_t paramIndex) const
  {
    Assert(paramIndex < m_parameters.size());
    return m_parameters[paramIndex];
  }

  void Parser::generateUsageString()
  {
    std::vector<std::string> flagNames;
    std::vector<std::string> flagDescriptions;
    std::vector<std::string> argumentStrings;

    size_t longestName = 0;

    // Initial command line
    for(auto &i : m_argumentDescriptions)
    {
      bool optional = (i.m_entryFlags | EntryFlags::Optional) != 0;
      std::string argString = " ";

      // Prologue
      if(optional)
        argString += "[";

      argString += m_switchChars[0];
      argString += i.m_shortName;

      // Generate usage name
      std::string flagName = m_switchChars.substr(0, 1) + i.m_shortName;
      if(!i.m_longName.empty())
      {
        flagName += (", " + m_switchChars.substr(0, 1)) + m_switchChars.substr(0, 1);
        flagName += i.m_longName;

        argString += ("|" + m_switchChars.substr(0, 1)) + m_switchChars.substr(0, 1) + i.m_longName;
      }


      switch(i.m_entryType)
      {
      case EntryType::Option:
        break;
      case EntryType::Parameter:
        argString += " <" + TypeToString(i.m_valueType) + ">";
        flagName += "=<" + TypeToString(i.m_valueType) + ">";
        break;
      case EntryType::Switch:
        break;
      case EntryType::UsageText:
        break;
      }

      // Epilogue
      if(optional)
        argString += "]";

      flagDescriptions.push_back(i.m_description);
      flagNames.push_back(flagName);
      argumentStrings.push_back(argString);

      size_t flagLength = flagName.length();
      if(flagLength > longestName)
        longestName = flagLength;
    }

    std::stringstream usageStream;
    usageStream << "Usage: " + m_applicationName;
    for(auto &i : argumentStrings)
      usageStream << i + " ";

    usageStream << std::endl;
    for(unsigned i = 0; i < flagNames.size(); ++i)
    {
      usageStream << std::left << std::setw(longestName) << flagNames[i] << "\t" << flagDescriptions[i] << std::endl;
    }

    m_usageString = usageStream.str();
  }

  const std::string &Parser::GetUsageString() const
  {
    return m_usageString;
  }

}

