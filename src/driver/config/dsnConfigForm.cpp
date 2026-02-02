#include "dsnConfigForm.hpp"

#include "../../util/stringFromChar.hpp"
#include "../../util/writeLog.hpp"
#include "win32controls/buttonMaker.hpp"
#include "win32controls/comboboxMaker.hpp"
#include "win32controls/editMaker.hpp"
#include "win32controls/labelMaker.hpp"

#include <CommCtrl.h>

// Define some identifiers for buttons and fields in the edit form.
constexpr int ID_EDIT_DSN                       = 101;
constexpr int ID_EDIT_HOSTNAME                  = 102;
constexpr int ID_EDIT_PORT                      = 103;
constexpr int ID_BUTTON_SAVE                    = 104;
constexpr int ID_BUTTON_CANCEL                  = 105;
constexpr int ID_COMBO_LOGLEVEL                 = 106;
constexpr int ID_COMBO_AUTHMETHOD               = 107;
constexpr int ID_EDIT_OIDC_DISC                 = 108;
constexpr int ID_EDIT_CLIENT_ID                 = 109;
constexpr int ID_EDIT_CLIENT_SECRET             = 110;
constexpr int ID_EDIT_OIDC_SCOPE                = 111;
constexpr int ID_STATIC_OIDC_DISC               = 112;
constexpr int ID_STATIC_CLIENT_ID               = 113;
constexpr int ID_STATIC_CLIENT_SECRET           = 114;
constexpr int ID_STATIC_OIDC_SCOPE              = 115;
constexpr int ID_EDIT_USERNAME                  = 116;
constexpr int ID_STATIC_USERNAME                = 117;
constexpr int ID_COMBO_OIDC_ENDPOINT_TYPE       = 118;
constexpr int ID_EDIT_AUTHORIZATION_ENDPOINT    = 119;
constexpr int ID_STATIC_AUTHORIZATION_ENDPOINT  = 120;
constexpr int ID_EDIT_TOKEN_ENDPOINT            = 121;
constexpr int ID_STATIC_TOKEN_ENDPOINT          = 122;
constexpr int ID_STATIC_OIDC_ENDPOINT_TYPE      = 123;
constexpr int ID_EDIT_PASSWORD                  = 124;
constexpr int ID_STATIC_PASSWORD                = 125;
constexpr int BUF_LEN                           = 1024;




std::string NOT_REQUIRED = "<Not Required>";

// Simple constructor that saves a copy of the parent window handle.
DSNForm::DSNForm(HWND parent, std::map<std::string, std::string> attributes) {
  this->parent = parent;
  if (attributes.count(CONFIG_FIELD_DSN_UPPER) > 0) {
    this->configResult.setDSN(attributes.at(CONFIG_FIELD_DSN_UPPER));
  }
  if (attributes.count(CONFIG_FIELD_HOSTNAME) > 0) {
    this->configResult.setHostname(attributes.at(CONFIG_FIELD_HOSTNAME));
  }
  if (attributes.count(CONFIG_FIELD_PORT) > 0) {
    this->configResult.setPort(attributes.at(CONFIG_FIELD_PORT));
  }
  if (attributes.count(CONFIG_FIELD_LOGLEVEL) > 0) {
    this->configResult.setLogLevel(attributes.at(CONFIG_FIELD_LOGLEVEL));
  }
  if (attributes.count(CONFIG_FIELD_AUTHMETHOD) > 0) {
    this->configResult.setAuthMethod(attributes.at(CONFIG_FIELD_AUTHMETHOD));
  }
  if (attributes.count(CONFIG_FIELD_OIDCDISCOVERYURL) > 0) {
    this->configResult.setOidcDiscoveryUrl(attributes.at(CONFIG_FIELD_OIDCDISCOVERYURL));
  }
  if (attributes.count(CONFIG_FIELD_CLIENTID) > 0) {
    this->configResult.setClientId(attributes.at(CONFIG_FIELD_CLIENTID));
  }
  if (attributes.count(CONFIG_FIELD_CLIENTSECRET) > 0) {
    this->configResult.setClientSecret(
        attributes.at(CONFIG_FIELD_CLIENTSECRET));
  }
  if (attributes.count(CONFIG_FIELD_OIDCSCOPE) > 0) {
    this->configResult.setOidcScope(attributes.at(CONFIG_FIELD_OIDCSCOPE));
  }
  if (attributes.count(CONFIG_FIELD_GRANTTYPE) > 0) {
    this->configResult.setGrantType(attributes.at(CONFIG_FIELD_GRANTTYPE));
  }
  if (attributes.count(CONFIG_FIELD_TOKENENDPOINT) > 0) {
    this->configResult.setTokenEndpoint(attributes.at(CONFIG_FIELD_TOKENENDPOINT));
  }
  if (attributes.count(CONFIG_FIELD_AUTHENDPOINT) > 0) {
    this->configResult.setAuthEndpoint(attributes.at(CONFIG_FIELD_AUTHENDPOINT));
  }
  if (attributes.count(CONFIG_FIELD_USERNAME) > 0) {
    this->configResult.setUsername(attributes.at(CONFIG_FIELD_USERNAME));
  }
  if (attributes.count(CONFIG_FIELD_PASSWORD) > 0) {
    this->configResult.setPassword(attributes.at(CONFIG_FIELD_PASSWORD));
  }
  if (attributes.count(CONFIG_FIELD_OIDCDISCOVERYMETHOD) > 0) {
    this->configResult.setOidcEndpointMethod(
        attributes.at(CONFIG_FIELD_OIDCDISCOVERYMETHOD));
  }
}
 
void UpdateDataFields(HWND hwnd)
{

  char buf[BUF_LEN]          = {0};
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
  HWND oidcMethodLabel       = GetDlgItem(hwnd, ID_STATIC_OIDC_ENDPOINT_TYPE);
  HWND oidcMethodItem        = GetDlgItem(hwnd, ID_COMBO_OIDC_ENDPOINT_TYPE);
  HWND tokenEndpointLabel    = GetDlgItem(hwnd, ID_STATIC_TOKEN_ENDPOINT);
  HWND tokenEndpointItem     = GetDlgItem(hwnd, ID_EDIT_TOKEN_ENDPOINT);
  HWND authEndpointLabel = GetDlgItem(hwnd, ID_STATIC_AUTHORIZATION_ENDPOINT);
  HWND authEndpointItem  = GetDlgItem(hwnd, ID_EDIT_AUTHORIZATION_ENDPOINT);
  HWND passwordLabel     = GetDlgItem(hwnd, ID_STATIC_PASSWORD);
  HWND passwordItem      = GetDlgItem(hwnd, ID_EDIT_PASSWORD);
  HWND userNameLabel     = GetDlgItem(hwnd, ID_STATIC_PASSWORD);
  HWND userNameItem      = GetDlgItem(hwnd, ID_EDIT_USERNAME);

  LRESULT authIndex = SendMessage(authMethodItem, CB_GETCURSEL, 0, 0);
  SendMessage(authMethodItem, CB_GETLBTEXT, authIndex, (LPARAM)buf);
  std::string authMethod  = stringFromChar(buf, CHAR_IS_NTS);
  LRESULT oidcMethodIndex = SendMessage(oidcMethodItem, CB_GETCURSEL, 0, 0);
  SendMessage(oidcMethodItem, CB_GETLBTEXT, oidcMethodIndex, (LPARAM)buf);
  std::string oidcMethod = stringFromChar(buf, CHAR_IS_NTS);

  auto enableUserName         = false;
  auto enablePassword         = false;
  auto enableClientId         = false;
  auto enableClientSecret     = false;
  auto enableOidcScope        = false;
  auto enableOidcDiscoveryUrl = false;
  auto enableTokenEndpoint    = false;
  auto enableAuthEndpoint     = false;
  auto enableOidcMethod       = false;


  if (authMethod == AUTH_NAME_CLIENT_CRED_AUTH ||
      authMethod == AUTH_NAME_DEVICE_CODE) {
    enableClientId     = true;
    enableClientSecret = true;
    enableOidcMethod   = true;
    enableOidcScope    = true;
    if (oidcMethod == OIDC_ENDPOINT_METHOD_NAME_URL) {
      enableOidcDiscoveryUrl = true;

    } else {
      enableAuthEndpoint  = true;
      enableTokenEndpoint = true;
    }
  } else if (authMethod == AUTH_NAME_USERNAME_AND_PASSWORD) {
    enableUserName = true;
    enablePassword = true;
  }

 





  EnableWindow(userNameItem, enableUserName);
  EnableWindow(passwordItem, enablePassword);
  EnableWindow(clientIdItem, enableClientId);
  EnableWindow(clientSecretItem, enableClientSecret);
  EnableWindow(oidcScopeItem, enableOidcScope);
  EnableWindow(oidcDiscoveryUrlItem, enableOidcDiscoveryUrl);
  EnableWindow(tokenEndpointItem, enableTokenEndpoint);
  EnableWindow(authEndpointItem, enableAuthEndpoint);
  EnableWindow(oidcMethodItem, enableOidcMethod);
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
      HWND oidcMethodLabel       = GetDlgItem(hwnd, ID_STATIC_OIDC_ENDPOINT_TYPE);
      HWND oidcMethodItem        = GetDlgItem(hwnd, ID_COMBO_OIDC_ENDPOINT_TYPE);
      HWND tokenEndpointLabel = GetDlgItem(hwnd, ID_STATIC_TOKEN_ENDPOINT);
      HWND tokenEndpointItem    = GetDlgItem(hwnd, ID_EDIT_TOKEN_ENDPOINT);
      HWND authEndpointLabel    = GetDlgItem(hwnd, ID_STATIC_AUTHORIZATION_ENDPOINT);
      HWND authEndpointItem     = GetDlgItem(hwnd, ID_EDIT_AUTHORIZATION_ENDPOINT);
      HWND passwordLabel =  GetDlgItem(hwnd, ID_STATIC_PASSWORD);
      HWND passwordItem = GetDlgItem(hwnd, ID_EDIT_PASSWORD);
      HWND userNameLabel = GetDlgItem(hwnd, ID_STATIC_PASSWORD);
      HWND userNameItem  = GetDlgItem(hwnd, ID_EDIT_USERNAME);




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

          //Handle the Username
          GetWindowText(userNameItem, buf, BUF_LEN);
          driverConfigPtr->setUsername(std::string(buf));

          //Handle the password
          GetWindowText(passwordItem, buf, BUF_LEN);
          driverConfigPtr->setPassword(std::string(buf));

          //handle the token endpoint
          GetWindowText(tokenEndpointItem, buf, BUF_LEN);
          driverConfigPtr->setTokenEndpoint(std::string(buf));

          
          // handle the auth endpoint
          GetWindowText(authEndpointItem, buf, BUF_LEN);
          driverConfigPtr->setAuthEndpoint(std::string(buf));


          // Handle the oidc  method
          LRESULT oidcMethodIndex = SendMessage(oidcMethodItem, CB_GETCURSEL, 0, 0);
          SendMessage(
              oidcMethodItem, CB_GETLBTEXT, oidcMethodIndex, (LPARAM)buf);
          driverConfigPtr->setOidcEndpointMethod(std::string(buf));


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
          //reset dialog box
          UpdateDataFields(hwnd);

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
                             600,
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
  labelMaker( form, 280, ID_STATIC_OIDC_ENDPOINT_TYPE, "OIDC Enpoint Type:", true);
  labelMaker(
      form, 310, ID_STATIC_TOKEN_ENDPOINT, "Token Endpoint:", true);
  labelMaker(
      form, 340, ID_STATIC_AUTHORIZATION_ENDPOINT, "Auth Endpoint:", true);
  labelMaker(form, 370, ID_STATIC_USERNAME, "User Name:", true);
  labelMaker(form, 400, ID_STATIC_PASSWORD, "Password:", true);


  WriteLog(LL_TRACE, "  Creating Text Entries");
  HWND hwndDsn              = editMaker(form, 10, ID_EDIT_DSN, visible);
  HWND hwndHostname         = editMaker(form, 40, ID_EDIT_HOSTNAME, visible);
  HWND hwndPort             = editMaker(form, 70, ID_EDIT_PORT, visible);
  HWND hwndOidcDiscoveryUrl = editMaker(form, 160, ID_EDIT_OIDC_DISC, oidcVis);
  HWND hwndClientId         = editMaker(form, 190, ID_EDIT_CLIENT_ID, oidcVis);
  HWND hwndClientSecret = editMaker(form, 220, ID_EDIT_CLIENT_SECRET, oidcVis);
  HWND hwndOidcScope    = editMaker(form, 250, ID_EDIT_OIDC_SCOPE, oidcVis);
  HWND hwndTokenEndpoint       = editMaker(form, 310, ID_EDIT_TOKEN_ENDPOINT, true);
  HWND hwndAuthEndpoint       = editMaker(form, 340, ID_EDIT_AUTHORIZATION_ENDPOINT, true);
  HWND hwndUsername        = editMaker(form, 370, ID_EDIT_USERNAME, true);
  HWND hwndPassword       = editMaker(form, 400, ID_EDIT_PASSWORD, true);

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

  
 // WriteLog(LL_TRACE, "  Creating OIDC Method Combobox");
 HWND hwndOidcMethod =
     comboboxMaker(form, 280, ID_COMBO_OIDC_ENDPOINT_TYPE, OIDC_METHOD_NAMES);

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
  setCombobox(hwndOidcMethod, this->configResult.getOidcEndpointMethodStr());

  WriteLog(LL_TRACE, "  Creating Buttons");
  HWND hwndSave   = buttonMaker(form, 160, ID_BUTTON_SAVE, "Save");
  HWND hwndCancel = buttonMaker(form, 260, ID_BUTTON_CANCEL, "Cancel");
  UpdateDataFields(form);
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
