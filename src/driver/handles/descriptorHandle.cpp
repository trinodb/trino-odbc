#include "descriptorHandle.hpp"
#include "../../util/writeLog.hpp"


Descriptor::Descriptor(SQLSMALLINT columnCount) {
  this->fields.resize(columnCount);
  // We may need to manage memory allocation/deallocation
  // for these, but maybe if we leave them all as nullptrs
  // it will just work. Let's try it, but leave the code
  // here just in case.
  // this->Field_ArrayStatusPtr    = new SQLUSMALLINT[1];
  // this->Field_ArrayStatusPtr[0] = SQL_ROW_NOROW;
  // this->Field_BindOffsetPtr     = new SQLLEN[1];
  // this->Field_BindOffsetPtr     = 0;
}

Descriptor::~Descriptor() {
  // delete[] this->Field_ArrayStatusPtr;
  // delete[] this->Field_BindOffsetPtr;
}

void Descriptor::setField(SQLSMALLINT columnIndex, DescriptorField& field) {
  if (columnIndex >= static_cast<SQLSMALLINT>(this->fields.size())) {
    // It's entirely possible that the ODBC driver will try to bind
    // a column before Trino has returned any column information to us.
    // That's okay, we'll just resize the fields array to accomodate.
    // We need to leave room for the bookmark column on the front.
    this->fields.resize(columnIndex + 1);
  }
  this->fields[columnIndex] = field;
}

DescriptorField Descriptor::getField(SQLSMALLINT columnIndex) {
  if (columnIndex < 0 ||
      columnIndex >= static_cast<SQLSMALLINT>(this->fields.size())) {
    return DescriptorField();
  }
  return this->fields[columnIndex];
}

const DescriptorField& Descriptor::getFieldRef(SQLSMALLINT columnIndex) {
  // We shouldn't ask for a field by reference before we've set up the
  // descriptors. Hopefully this holds true.
  return this->fields[columnIndex];
}

void Descriptor::resize(SQLSMALLINT newSize) {
  this->fields.resize(newSize);
}

void Descriptor::reset() {
  this->fields.resize(0);
}

SQLSMALLINT Descriptor::getColumnCount() {
  return static_cast<SQLSMALLINT>(this->fields.size());
}
