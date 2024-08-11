#pragma once

#include "../../../util/windowsLean.hpp"
#include <string>

HWND labelMaker(HWND form,
                int vertOffset,
                int labelId,
                std::string labelText,
                bool visible);
