#include "CLA.h"

#include <iostream>

int main(int argc, const char **argv)
{
  CLA::Parser parser({
    { CLA::EntryType::Option, "h", "host",    "IP/URL and port for the IRC server", CLA::ValueType::String, CLA::EntryFlags::Manditory },
    { CLA::EntryType::Option, "u", "user",    "Username for the IRC client",        CLA::ValueType::String, CLA::EntryFlags::Manditory },
    { CLA::EntryType::Option, "p", "pass",    "Password for the IRC server",        CLA::ValueType::String, CLA::EntryFlags::Manditory },
    { CLA::EntryType::Switch, "v", "",        "Verbose output mode",                CLA::ValueType::None,   CLA::EntryFlags::Optional }
  });

  CLA::ParseResult result = parser.Parse(argc, argv);
  if(result != CLA::ParseResult::Valid)
  {
    std::cout << parser.GetError() << std::endl;
    std::cout << parser.GetUsageString();
    return EXIT_FAILURE;
  }

  std::string host;
  std::string username;
  std::string password;
  bool        verbose = false;

  parser.Found("host", host);
  parser.Found("user", username);
  parser.Found("pass", password);
  verbose = parser.FoundSwitch("v");

  std::cout << "IRC Client created!" << std::endl;
  std::cout << "Connecting to " << host << " with password '" << password <<  "'..." << std::endl;
  std::cout << "Username: " << username << std::endl;
  std::cout << "Verbose Mode: " << std::boolalpha << verbose << std::endl;

  return EXIT_SUCCESS;
}