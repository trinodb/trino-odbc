#include "labelMaker.hpp"

HWND labelMaker(HWND form,
                int vertOffset,
                int labelId,
                std::string labelText,
                bool visible) {
  DWORD visibleStyle = visible ? WS_VISIBLE : 0;
  return CreateWindow("STATIC",
                      labelText.c_str(),
                      visibleStyle | WS_CHILD,
                      10,
                      vertOffset,
                      145,
                      22,
                      form,
                      reinterpret_cast<HMENU>(static_cast<UINT_PTR>(labelId)),
                      GetModuleHandle(NULL),
                      NULL);
}
