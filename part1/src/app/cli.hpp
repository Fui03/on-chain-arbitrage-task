// Public CLI helpers for parsing detector options and printing usage text.

#ifndef ARB_APP_CLI_HPP
#define ARB_APP_CLI_HPP

#include "domain/model/models.hpp"

#include <iosfwd>

/// Parses CLI flags into a validated `SearchOptions` struct.
SearchOptions ParseCliArgs(int argc, char** argv);

/// Prints CLI usage information to the provided stream.
void PrintUsage(std::ostream& output);

#endif
