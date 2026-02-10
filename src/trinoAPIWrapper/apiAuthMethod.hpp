#pragma once

enum ApiAuthMethod {
  AM_NO_AUTH          = 0,
  AM_EXTERNAL_AUTH    = 1,
  AM_CLIENT_CRED_AUTH = 2,
  AM_DEVICE_CODE      = 3,
  AM_USERNAME_AND_PASSWORD = 4,
};
