#pragma once

#include <string>

namespace CLA
{
  // Returns a copy of a lowered string
  std::string ToLower(const std::string &str);

  // Finds all instances of "find" and replaces it with "replaceWith"
  void ReplaceCharacters(std::string& str, char find, char replaceWith);

  // Extracts the filename and extension out of a path
  std::string ExtractFilename(const std::string& path);
}