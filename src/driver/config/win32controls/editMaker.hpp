#pragma once

#include "../../../util/windowsLean.hpp"
#include <string>

HWND editMaker(HWND form, int vertOffset, int editId, bool visible);

std::string getEditText(HWND edit);

void setEditText(HWND edit, std::string text);

void setEditReadOnly(HWND edit);

void setEditWriteable(HWND edit);
