#include "constants.hpp"

#ifdef DEBUG
std::string TEST_DSN = "TrinoTestDebug";
#else
std::string TEST_DSN = "TrinoTestRelease";
#endif
