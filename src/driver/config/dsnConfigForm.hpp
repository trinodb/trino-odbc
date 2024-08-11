#pragma once

#include "../../util/windowsLean.hpp"
#include <map>
#include <string>

#include "driverConfig.hpp"

class DSNForm {
  private:
    HWND parent;
    DriverConfig configResult;
    bool readOnlyDSN = false;

  public:
    DSNForm(HWND parent, std::map<std::string, std::string> attributes);
    void ShowDSNForm();
    DriverConfig getResult();
    void updateConfig(DriverConfig);
    void setReadOnlyDSN();
};
