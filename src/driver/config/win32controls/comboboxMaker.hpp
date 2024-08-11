#pragma once

#include "../../../util/windowsLean.hpp"
#include <string>
#include <vector>

HWND comboboxMaker(HWND form,
                   int vertOffset,
                   int comboboxId,
                   std::vector<std::string> labelNames);

void setCombobox(HWND comboBox, std::string valueStr);
