#pragma once

#include "windowsLean.hpp"
#include <sql.h>
#include <sqlext.h>

#include <string>

/*
Write a string to the buffer at InfoValuePtr,
and the length of the string to StringLengthPtr.

If InfoValuePtr is null, this still writes a length to
StringLengthPtr anyway. This is used by applications that
want to check the required size of an array before
allocating one.
*/
template <class T>
void writeNullTermStringToPtr(SQLPOINTER InfoValuePtr,
                              std::string s,
                              T* StringLengthPtr) {
  rsize_t length           = s.length();
  rsize_t nullcharPosition = length + 1;
  if (InfoValuePtr) {
    char* infoCharPtr = reinterpret_cast<char*>(InfoValuePtr);
    // strcpy_s is documented to write a null terminator to the
    // destination character array.
    strcpy_s(infoCharPtr, nullcharPosition, s.c_str());
  }
  if (StringLengthPtr) {
    *StringLengthPtr = static_cast<T>(length);
  }
}
