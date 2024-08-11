#include "comboboxMaker.hpp"

HWND comboboxMaker(HWND form,
                   int vertOffset,
                   int comboboxId,
                   std::vector<std::string> labelNames) {
  // Include WS_TABSTOP to enable tab navigation to the combobox.
  HWND hwndCombobox =
      CreateWindow("COMBOBOX",
                   "",
                   WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_TABSTOP,
                   165,
                   vertOffset,
                   415,
                   20,
                   form,
                   (HMENU)(UINT_PTR)comboboxId,
                   GetModuleHandle(NULL),
                   NULL);

  for (std::string labelName : labelNames) {
    SendMessage(hwndCombobox, CB_ADDSTRING, 0, (LPARAM)labelName.c_str());
  }
  return hwndCombobox;
}

void setCombobox(HWND comboBox, std::string valueStr) {
  if (!valueStr.empty()) {
    LRESULT index =
        SendMessage(comboBox, CB_FINDSTRINGEXACT, -1, (LPARAM)valueStr.c_str());
    if (index != CB_ERR) {
      SendMessage(comboBox, CB_SETCURSEL, index, 0);
    }
  }
}
