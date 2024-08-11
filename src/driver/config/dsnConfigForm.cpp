#include "dsnConfigForm.hpp"

#include "../../util/stringFromChar.hpp"
#include "../../util/writeLog.hpp"
#include "win32controls/buttonMaker.hpp"
#include "win32controls/comboboxMaker.hpp"
#include "win32controls/editMaker.hpp"
#include "win32controls/labelMaker.hpp"

#include <CommCtrl.h>

// Define some identifiers for buttons and fields in the edit form.
constexpr int ID_EDIT_DSN             = 101;
constexpr int ID_EDIT_HOSTNAME        = 102;
constexpr int ID_EDIT_PORT            = 103;
constexpr int ID_BUTTON_SAVE          = 104;
constexpr int ID_BUTTON_CANCEL        = 105;
constexpr int ID_COMBO_LOGLEVEL       = 106;
constexpr int ID_COMBO_AUTHMETHOD     = 107;
constexpr int ID_EDIT_OIDC_DISC       = 108;
constexpr int ID_EDIT_CLIENT_ID       = 109;
constexpr int ID_EDIT_CLIENT_SECRET   = 110;
constexpr int ID_EDIT_OIDC_SCOPE      = 111;
constexpr int ID_STATIC_OIDC_DISC     = 112;
constexpr int ID_STATIC_CLIENT_ID     = 113;
constexpr int ID_STATIC_CLIENT_SECRET = 114;
constexpr int ID_STATIC_OIDC_SCOPE    = 115;
constexpr int BUF_LEN                 = 1024;


std::string NOT_REQUIRED = "<Not Required>";

// Simple constructor that saves a copy of the parent window handle.
DSNForm::DSNForm(HWND parent, std::map<std::string, std::string> attributes) {
  this->parent = parent;
  if (attributes.count("DSN") > 0) {
    this->configResult.setDSN(attributes.at("DSN"));
  }
  if (attributes.count("hostname") > 0) {
    this->configResult.setHostname(attributes.at("hostname"));
  }
  if (attributes.count("port") > 0) {
    this->configResult.setPort(attributes.at("port"));
  }
  if (attributes.count("loglevel") > 0) {
    this->configResult.setLogLevel(attributes.at("loglevel"));
  }
  if (attributes.count("authmethod") > 0) {
    this->configResult.setAuthMethod(attributes.at("authmethod"));
  }
  if (attributes.count("oidcDiscoveryUrl") > 0) {
    this->configResult.setOidcDiscoveryUrl(attributes.at("oidcDiscoveryUrl"));
  }
  if (attributes.count("clientId") > 0) {
    this->configResult.setClientId(attributes.at("clientId"));
  }
  if (attributes.count("clientSecret") > 0) {
    this->configResult.setClientSecret(attributes.at("clientSecret"));
  }
  if (attributes.count("oidcScope") > 0) {
    this->configResult.setOidcScope(attributes.at("oidcScope"));
  }
}


LRESULT CALLBACK WINDOW_CB(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // This is the window "procedure" for the DSN Config window, which is
  // essentially a callback function. This is where we interact with the window
  // to extract values from it.
  DriverConfig* driverConfigPtr = nullptr;
  if (uMsg == WM_CREATE) {
    CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
    driverConfigPtr = reinterpret_cast<DriverConfig*>(pCreate->lpCreateParams);
    SetWindowLongPtr(
        hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(driverConfigPtr));
  } else {
    driverConfigPtr =
        reinterpret_cast<DriverConfig*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
  }

  switch (uMsg) {
    case WM_COMMAND: {
      WriteLog(LL_TRACE, "  WM_COMMAND executing");
      HWND dsnItem               = GetDlgItem(hwnd, ID_EDIT_DSN);
      HWND hostnameItem          = GetDlgItem(hwnd, ID_EDIT_HOSTNAME);
      HWND portItem              = GetDlgItem(hwnd, ID_EDIT_PORT);
      HWND logLevelItem          = GetDlgItem(hwnd, ID_COMBO_LOGLEVEL);
      HWND authMethodItem        = GetDlgItem(hwnd, ID_COMBO_AUTHMETHOD);
      HWND oidcDiscoveryUrlItem  = GetDlgItem(hwnd, ID_EDIT_OIDC_DISC);
      HWND oidcDiscoveryUrlLabel = GetDlgItem(hwnd, ID_STATIC_OIDC_DISC);
      HWND clientIdItem          = GetDlgItem(hwnd, ID_EDIT_CLIENT_ID);
      HWND clientIdLabel         = GetDlgItem(hwnd, ID_STATIC_CLIENT_ID);
      HWND clientSecretItem      = GetDlgItem(hwnd, ID_EDIT_CLIENT_SECRET);
      HWND clientSecretLabel     = GetDlgItem(hwnd, ID_STATIC_CLIENT_SECRET);
      HWND oidcScopeItem         = GetDlgItem(hwnd, ID_EDIT_OIDC_SCOPE);
      HWND oidcScopeLabel        = GetDlgItem(hwnd, ID_STATIC_OIDC_SCOPE);
      char buf[BUF_LEN]          = {0};
      switch (LOWORD(wParam)) {
        case ID_BUTTON_SAVE: {
          WriteLog(LL_TRACE, "  ID_BUTTON_SAVE executing");

          // Handle the DSN text
          GetWindowText(dsnItem, buf, BUF_LEN);
          driverConfigPtr->setDSN(std::string(buf));

          // Handle the hostname
          GetWindowText(hostnameItem, buf, BUF_LEN);
          driverConfigPtr->setHostname(std::string(buf));

          // Handle the port
          GetWindowText(portItem, buf, BUF_LEN);
          driverConfigPtr->setPort(std::string(buf));

          // Handle the log level
          LRESULT logIndex = SendMessage(logLevelItem, CB_GETCURSEL, 0, 0);
          SendMessage(logLevelItem, CB_GETLBTEXT, logIndex, (LPARAM)buf);
          driverConfigPtr->setLogLevel(std::string(buf));

          // Handle the auth method
          LRESULT authIndex = SendMessage(authMethodItem, CB_GETCURSEL, 0, 0);
          SendMessage(authMethodItem, CB_GETLBTEXT, authIndex, (LPARAM)buf);
          driverConfigPtr->setAuthMethod(std::string(buf));

          // Handle the OIDC Discovery URL
          GetWindowText(oidcDiscoveryUrlItem, buf, BUF_LEN);
          driverConfigPtr->setOidcDiscoveryUrl(std::string(buf));

          // Handle the Client ID
          GetWindowText(clientIdItem, buf, BUF_LEN);
          driverConfigPtr->setClientId(std::string(buf));

          // Handle the Client Secret
          GetWindowText(clientSecretItem, buf, BUF_LEN);
          driverConfigPtr->setClientSecret(std::string(buf));

          // Handle the OIDC Scope
          GetWindowText(oidcScopeItem, buf, BUF_LEN);
          driverConfigPtr->setOidcScope(std::string(buf));

          driverConfigPtr->setIsSaved(true);

          DestroyWindow(hwnd);
          break;
        }
        case ID_BUTTON_CANCEL: {
          WriteLog(LL_TRACE, "  ID_BUTTON_CANCEL executing");
          driverConfigPtr->setIsSaved(false);

          DestroyWindow(hwnd);
          break;
        }
        default: {
          WriteLog(LL_TRACE, "  WINDOW_CB default (redraw)");
          LRESULT authIndex = SendMessage(authMethodItem, CB_GETCURSEL, 0, 0);
          SendMessage(authMethodItem, CB_GETLBTEXT, authIndex, (LPARAM)buf);
          std::string authMethod = stringFromChar(buf, CHAR_IS_NTS);
          if (authMethod == "OIDC Client Cred Auth") {
            WriteLog(LL_TRACE, "  Client Cred Auth");
            // Show the OIDC Config fields
            if (getEditText(oidcDiscoveryUrlItem) == NOT_REQUIRED) {
              setEditText(oidcDiscoveryUrlItem, "");
            }
            setEditWriteable(oidcDiscoveryUrlItem);
            if (getEditText(clientIdItem) == NOT_REQUIRED) {
              setEditText(clientIdItem, "");
            }
            setEditWriteable(clientIdItem);
            if (getEditText(clientSecretItem) == NOT_REQUIRED) {
              setEditText(clientSecretItem, "");
            }
            setEditWriteable(clientSecretItem);
            if (getEditText(oidcScopeItem) == NOT_REQUIRED) {
              setEditText(oidcScopeItem, "");
            }
            setEditWriteable(oidcScopeItem);
            InvalidateRect(hwnd, NULL, FALSE);
          } else if (not authMethod.empty()) {
            WriteLog(LL_TRACE, "  Other Auth");
            // Hide the OIDC Config fields
            if (getEditText(oidcDiscoveryUrlItem) == "") {
              setEditText(oidcDiscoveryUrlItem, NOT_REQUIRED);
            }
            setEditReadOnly(oidcDiscoveryUrlItem);
            if (getEditText(clientIdItem) == "") {
              setEditText(clientIdItem, NOT_REQUIRED);
            }
            setEditReadOnly(clientIdItem);
            if (getEditText(clientSecretItem) == "") {
              setEditText(clientSecretItem, NOT_REQUIRED);
            }
            setEditReadOnly(clientSecretItem);
            if (getEditText(oidcScopeItem) == "") {
              setEditText(oidcScopeItem, NOT_REQUIRED);
            }
            setEditReadOnly(oidcScopeItem);
            InvalidateRect(hwnd, NULL, FALSE);
          }
          break;
        }
      }
      break;
    }
    case WM_DESTROY: {
      WriteLog(LL_TRACE, "  WM_DESTROY executing");
      PostQuitMessage(0);
      break;
    }
    default: {
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
  }
  return 0;
}

void DSNForm::ShowDSNForm() {
  WriteLog(LL_DEBUG, "  Creating DSN Form");
  const char className[] = "FormWindowClass";

  WNDCLASS wc      = {};
  wc.lpfnWndProc   = WINDOW_CB;
  wc.hInstance     = GetModuleHandle(NULL);
  wc.lpszClassName = className;
  // Getting data back from a window callback is tricky. The easiest way
  // to do it is to share a pointer.
  wc.cbWndExtra = sizeof(DriverConfig*);

  WriteLog(LL_TRACE, "  Registering class");
  RegisterClass(&wc);

  WriteLog(LL_TRACE, "  Creating window");
  HWND form    = CreateWindowEx(WS_EX_CLIENTEDGE,
                             className,
                             "Enter DSN Information",
                             WS_OVERLAPPED | WS_VISIBLE,
                             CW_USEDEFAULT,
                             CW_USEDEFAULT,
                             625,
                             400,
                             this->parent,
                             NULL,
                             GetModuleHandle(NULL),
                             &this->configResult);
  bool visible = true;
  // FEATURE: actually hide the OIDC fields if the user doesn't select OIDC
  // auth. For now, they're just disabled but still visible.
  bool oidcVis = true;

  WriteLog(LL_TRACE, "  Creating Labels");
  labelMaker(form, 10, NULL, "DSN Name:", visible);
  labelMaker(form, 40, NULL, "Hostname:", visible);
  labelMaker(form, 70, NULL, "Port:", visible);
  labelMaker(form, 100, NULL, "Log Level:", visible);
  labelMaker(form, 130, NULL, "Auth Method:", visible);
  labelMaker(form, 160, ID_STATIC_OIDC_DISC, "OIDC Discovery URL:", oidcVis);
  labelMaker(form, 190, ID_STATIC_CLIENT_ID, "Client ID:", oidcVis);
  labelMaker(form, 220, ID_STATIC_CLIENT_SECRET, "Client Secret:", oidcVis);
  labelMaker(form, 250, ID_STATIC_OIDC_SCOPE, "OIDC Scope:", oidcVis);

  WriteLog(LL_TRACE, "  Creating Text Entries");
  HWND hwndDsn              = editMaker(form, 10, ID_EDIT_DSN, visible);
  HWND hwndHostname         = editMaker(form, 40, ID_EDIT_HOSTNAME, visible);
  HWND hwndPort             = editMaker(form, 70, ID_EDIT_PORT, visible);
  HWND hwndOidcDiscoveryUrl = editMaker(form, 160, ID_EDIT_OIDC_DISC, oidcVis);
  HWND hwndClientId         = editMaker(form, 190, ID_EDIT_CLIENT_ID, oidcVis);
  HWND hwndClientSecret = editMaker(form, 220, ID_EDIT_CLIENT_SECRET, oidcVis);
  HWND hwndOidcScope    = editMaker(form, 250, ID_EDIT_OIDC_SCOPE, oidcVis);

  // Sometimes the DSN should be read-only such as when an existing DSN is being
  // configured. In those cases, we need to set the EDIT control to readonly as
  // well.
  if (this->readOnlyDSN) {
    setEditReadOnly(hwndDsn);
  }

  WriteLog(LL_TRACE, "  Creating Log Level Combobox");
  HWND hwndLogLevel =
      comboboxMaker(form, 100, ID_COMBO_LOGLEVEL, LOG_LEVEL_NAMES);

  WriteLog(LL_TRACE, "  Creating Auth Method Combobox");
  HWND hwndAuthMethod =
      comboboxMaker(form, 130, ID_COMBO_AUTHMETHOD, AUTH_METHOD_NAMES);

  WriteLog(LL_TRACE, "  Pre-populating the text fields");
  setEditText(hwndDsn, this->configResult.getDSN());
  setEditText(hwndHostname, this->configResult.getHostname());
  setEditText(hwndPort, this->configResult.getPortStr());
  setEditText(hwndOidcDiscoveryUrl, this->configResult.getOidcDiscoveryUrl());
  setEditText(hwndClientId, this->configResult.getClientId());
  setEditText(hwndClientSecret, this->configResult.getClientSecret());
  setEditText(hwndOidcScope, this->configResult.getOidcScope());

  WriteLog(LL_TRACE, "  Pre-poplating Comboboxes");
  setCombobox(hwndLogLevel, this->configResult.getLogLevelStr());
  setCombobox(hwndAuthMethod, this->configResult.getAuthMethodStr());

  WriteLog(LL_TRACE, "  Creating Buttons");
  HWND hwndSave   = buttonMaker(form, 160, ID_BUTTON_SAVE, "Save");
  HWND hwndCancel = buttonMaker(form, 260, ID_BUTTON_CANCEL, "Cancel");

  MSG msg = {};
  WriteLog(LL_TRACE, "  Polling...");
  while (GetMessage(&msg, NULL, 0, 0)) {
    if (!IsDialogMessage(form, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
  }
  WriteLog(LL_TRACE, "  Polling completed");

  WriteLog(LL_TRACE, "  Unregistering class");
  UnregisterClass(className, GetModuleHandle(NULL));
}

DriverConfig DSNForm::getResult() {
  return this->configResult;
}

void DSNForm::updateConfig(DriverConfig config) {
  this->configResult = config;
}

void DSNForm::setReadOnlyDSN() {
  this->readOnlyDSN = true;
}
