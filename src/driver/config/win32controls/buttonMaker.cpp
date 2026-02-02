#include "buttonMaker.hpp"

HWND buttonMaker(HWND form,
                 int horizOffset,
                 int buttonId,
                 std::string buttonText) {
  // Include WS_TABSTOP to enable tab navigation to buttons.
  return CreateWindow("BUTTON",
                      buttonText.c_str(),
                      WS_VISIBLE | WS_CHILD | WS_TABSTOP,
                      horizOffset,
                      500,
                      80,
                      30,
                      form,
                      reinterpret_cast<HMENU>(static_cast<UINT_PTR>(buttonId)),
                      GetModuleHandle(NULL),
                      NULL);
}
