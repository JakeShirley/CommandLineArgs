#include "CLAUtils.h"

// STL
#include <string>     // std::string
#include <algorithm>  // std::transform
#include <vector>     // std::vector
#include <sstream>    // std::stringstream
#include <functional> // stdd::not1, std::ptr_fun
#include <cctype>     // std::isspace

namespace CLA
{
  void ReplaceCharacters(std::string& str, char find, char replaceWith)
  {
    for(unsigned i = 0; i < str.length(); ++i)
      if(str[i] == find)
        str[i] = replaceWith;
  }

  std::string ExtractFilename(const std::string& path)
  {
    std::string result = path;

    ReplaceCharacters(result, '/', '\\');

    unsigned filenameStart = result.find_last_of('\\');

    filenameStart = (filenameStart == std::string::npos) ? 0 : filenameStart + 1;

    result.erase(0, filenameStart);

    return result;
  }

  std::string ToLower(const std::string &str)
  {
    std::string retValue(str);
    std::transform(retValue.begin(), retValue.end(), retValue.begin(), ::tolower);
    return retValue;
  }

}