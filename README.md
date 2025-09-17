# Project Overview: Trino ODBC Driver (Partial Implementation)

[![License](https://img.shields.io/badge/License-Apache_2.0-blue.svg)]()
[![Latest Release](https://img.shields.io/github/v/release/trinodb/trino-odbc.svg)]()
[![Build Status](https://img.shields.io/github/actions/workflow/status/trinodb/trino-odbc/make_release.yml.svg)]()
[![Contributors](https://img.shields.io/github/contributors/trinodb/trino-odbc.svg)]()
[![Code Style](https://img.shields.io/badge/Code%20Style-Clang%20Format-blue.svg)]()

## Description

This open-source project is a partially complete ODBC driver for the
[Trino distributed SQL engine](https://trino.io/). It implements the
essential portions of the ODBC core specification required to enable
Microsoft Excel and Microsoft PowerBI Desktop to connect to and
execute queries against a Trino server using either External
Authentication or OIDC Client Credential flow.
The driver was developed to address a narrow gap in connectivity
options for these tools on the Windows operating system within
the Open Source Trino community.

Note: this project is not sponsored by or affiliated with
Microsoft in any way.

### :warning: Disclaimer: Scope and Limitations :warning:

This driver is tested to work within a very narrow scope on Windows
PCs configured for use within our organization. Specifically, we target
compatibility with Microsoft Excel and PowerBI Desktop with External
Authentication and Client Credential Auth. Other tools, forms of
authentication, and functionality of the ODBC specification were
left unimplemented if not required by these tools. Functionality on
external systems or configurations may vary, and compatibility is
not guaranteed.  For users needing cross-platform compatibility or
full ODBC specification support, you should consider a feature-complete
commercially available ODBC driver for Trino.

### Alternatives and Performance Considerations

For PowerBI users, there is also the
[Trino PowerBI Connector](https://github.com/CreativeDataEU/PowerBITrinoConnector)
project, which works well for many use cases. In our experience,
it was found to exhibit slow loading performance on datasets contaning
hundreds of millions of rows of data. This appears to be
a limitation in the performance of the Microsoft Power Query
language, and is not a reflection on the quality of work
in the PowerBITrinoConnector project, which is fantastic!

### Contributing

Please see [our Contributing guide](./CONTRIBUTING.md) for more information.

### Known Limitations

- Only compiled for Microsoft Windows
- Only supports reading data, not writing/transacting data.
- Does not support most forms of Trino authentication including password authentication
- Does not support the ODBC wide-char unicode encoding (UCS-2 format, 16-bit characters)
- Does not support Parameterized queries (SQLBindParameter, SQLParamData, SQLNumParams etc.)
- Does not support ODBC conformance Level 1 or Level 2
  - [About Conformance Levels](https://learn.microsoft.com/en-us/sql/odbc/reference/develop-app/interface-conformance-levels)
  - It does not __completely__ support the Core conformance level, but is close.
- Does not support SQL Transactions (SQLEndTran)
- Does not support binding and fetching multiple rows in a single call (SQLFetch with array size greater than 1)
- Does not support non-sequential data fetching (SQLFetchScroll/SQLExtendedFetch)
- Does not support iteratively discovering and enumerating connection attributes (SQLBrowseConnect)
- Does not support prepared execution (SQLPrepare, SQLExecute)
- All columns are reported as being nullable, regardless of whether they are
  actually nullable or not.

There are many more limitations not mentioned here. The ODBC specification
is quite large and it's very difficult to tell what ODBC features a
given application will require by reading the spec. None of the above
features are required for Microsoft PowerBI Desktop or Microsoft Excel to
use the driver to read data from Trino using Client Credential Auth or
External Authentication.


### Identifying Specific Limitations

The best way to find what if anything is missing is to give it a try!
This is relatively easy to do.

1. Install this driver.
1. Configure a DSN for this driver and enable trace-level logging in the DSN config.
1. Enable tracing in the driver manager as well.
1. Trigger an error when your application tries to use this driver.
1. Review any error dialog boxes from the application, if they appear
1. Review the driver logs (C:\temp\odbclog.txt) for the word "error"
1. Review the ODBC Driver manager trace logs for the word "error"


From here, you'll often find precisely what features were required
but were not yet implemented. Once you know what's missing, a review
of the the ODBC specification's documentation typically explains
what the missing functionality accomplishes. From there, it's
just a matter of writing the code to enable it.


## Developer Prerequisites

We recommend using Visual Studio Community (Not VSCode) for developing
this project. Even in a commercial organization, Visual Studio Community
is free to use for contributing to open source projects such as this one:
https://visualstudio.microsoft.com/vs/community/.

This ODBC driver makes use of the vcpkg tool for CMake. Follow the instructions
[here](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-cmd)
to set it up.

The installer package makes use of WiX. Installation and configuration of WiX is
described in the "Installing" section below.

## About the ODBC paradigm

Most applications that use ODBC do so through a driver manager rather than interacting
directly with the driver. The driver manager is provided by the operating system or
an operating system package (like UnixODBC). The driver manager intercepts calls
from an application to execute queries, read data, describe columns, or otherwise.
Then the driver manager, with knowledge of the actual capabilties of any given
driver, translates those calls into the appropriate function calls for the
driver. It functions as a kind of normalizing intermediary. This allows
application writers not to need to understand the differences between the ODBC 2,
3.0, 3.5, and 3.8 specs to use an ODBC driver. They simply call the functions
they want to use, and either the driver implements enough to do it or it does not.

In order to enable this functionality in the driver, the driver's author must
supply information about the driver's capabilities to the driver manager. This
is done by implementing a pair of functions. The first is `SQLGetInfo`, which
provides a variety of information about the driver and its capabilities. One
of the things the driver manager can request from `SQLGetInfo` is the
`SQL_DRIVER_ODBC_VER`, which allows the driver manager to infer what level of
capabilities should be present in the driver. The second critical function
is `SQLGetFunctions`, which provides information to the driver manager
about the specific set of functions that are available from the driver.
Not every driver implements every single function in the ODBC spec, and
this driver is no exception. `SQLGetFunctions` provides a way for the
driver manager to ask if a given function is supported or not.

## Testing and Debugging

The best way to test the driver is to install the build output DLL as
the driver for your system. ODBC drivers are installed entirely by
manipulating the Windows Registry. Set the keys as described in
`install/RegisterTrinoODBC.reg`, but substituting the path to the
TrinoODBC.dll file generated in the build output directory. Once this
is done, you should see TrinoODBC show up in your ODBC Data Sources
window (for both 32 and 64 bit). Every time you build this tool,
the system driver is effectively updated because it points to
the build output. This makes for a rapid code -> build -> test
loop. Note that these registry keys are the same ones used by
the installed release driver, so following this process will replace
the driver installed in your system with the driver built by compiling
this code.

The recommended approach to testing the driver's code is to create separate
driver installations (by editing install/RegisterTrinoODBC.reg) pointing
to debug and release builds of the driver. Name the drivers `TrinoODBCDebug`
and `TrinoODBCRelease`. Then, create separate DSNs targeting those drivers
named  `TrinoTestDebug` and `TrinoTestRelease`. The test executable is
already configured to select between DSNs with those two names based on
the presence of the `DEBUG` preprocessor definition, meaning your tests
will automatically target the correct driver based on your build
configuration.

To get started, the easiest thing to do is to pick a processor
bitness and stick with it until your feature is developed. If you decide
to test both x64 and x86 builds, make sure the Driver/DSN you are testing
has a bitness that matches the test suite you wish to run. It's very easy
to get this mixed up and test the wrong driver, since the ODBC Driver Manager
is unaware that it's being used in a test suite and should target a specific
build output unless explicitly configured to do so.

The test suite assumes this DSN is configured for a vanilla
trino instance. Running a standalone copy of the
[Trino Docker Image](https://hub.docker.com/r/trinodb/trino)
provides a trino instance that is sufficient to run the suite.

Once you have a driver and DSN installed and configured,
run the `TestDriver.exe` executable to run the full test suite
for this driver.


## Installing a Windows ODBC Driver

There are three ways to install this driver.

1. Using a pre-built and released MSI installer (See Releases Page)
2. Compiling your own driver and building your own MSI installer for it.
3. Manual driver compilation and installation.

### Using a pre-built MSI installer

See the releases page for the most recent release of this driver.

### Building your own MSI installer

This project uses [WiX](https://wixtoolset.org/) to build an installer package for
this driver.

1. We recommend using the .NET SDK to install WiX on your system. If developing in
   Visual Studio the .NET SDK is an optional installation option.
   You can re-run the installer and select "Modify" if you didn't select this the
   first time.
    1. If you want to manually install the .NET SDK, visit
       https://learn.microsoft.com/en-us/dotnet/core/sdk
1. If you haven't already done so, configure a nuget package source for the
   .NET SDK. Instructions for Visual Studio:
    1. https://stackoverflow.com/questions/69045231/nuget-package-sources-missing
1. Begin by installing the WiX tool on your system using the .NET SDK.
    1. Run `dotnet tool install --global wix --version 5.0.2`
1. Add the WiX UI extension.
    1. `wix extension add -g WixToolset.UI.wixext@5.0.2`
1. Build the installer
    1. `cd install`
    1. `./build_x64_installer.ps1` (for a 64-bit installer)
    1. `./build_x86_installer.ps1` (for a 32-bit installer)
1. The finished `TrinoODBC_x64.msi` or `TrinoODBC_x86.msi` installers will appear,
   ready to be used.


### Manual Install

1. Open regedit as admin
2. Add Driver Information:
    1. Navigate to HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBCINST.INI\ODBC Drivers.
    1. Right-click on ODBC Drivers, select New > String Value.
    1. Name the new string value with the name of your driver (e.g., TrinoODBC).
    1. Set its value to Installed.
3. Add Driver Configuration:
    1. Navigate to HKEY_LOCAL_MACHINE\SOFTWARE\ODBC\ODBCINST.INI.
    1. Right-click on ODBCINST.INI, select New > Key.
    1. Name the new key with the name of your driver (e.g., TrinoODBC).
    1. Under this new key, create the string values described in `install/RegisterTrinoODBC.reg`
    1. Driver: Full path to your driver DLL (e.g., C:\Path\To\TrinoODBC.dll).
    1. Setup: (optional) Full path to a setup DLL if you have one, or set it the same as the Driver.

# Dependencies

The following are upstream dependencies of the TrinoODBC driver. None of these dependencies
are included in source form in this repository - we recommend that you obtain them using the
vcpkg tool. All of them are used in unmodified form. Information about dependency
license distribution and compliance is also included below.

### Driver Runtime Dependencies

The driver's runtime dependencies are packaged and distributed when installing via the MSI
installer packages. Their licenses are available in `install\third_party_licenses.txt`, which is
automatically copied alongside the driver dll and the static libraries distributed with it.
After installation, you can find them at `[SystemFolder]\TrinoODBC\third_party_licenses.txt`
or `[System64Folder]\TrinoODBC\third_party_licenses.txt` depending on whether you are installing
a 32 or 64-bit driver.

1. [libcurl](https://curl.se/libcurl/)
    * [MIT License](https://github.com/Araq/libcurl/blob/master/LICENSE.txt)
1. [nlohmann JSON](https://github.com/nlohmann/json)
    * [MIT License](https://github.com/nlohmann/json/blob/develop/LICENSE.MIT)
1. [openssl](https://www.openssl.org/)
    * [Apache 2.0 License](https://github.com/openssl/openssl/blob/master/LICENSE.txt)
1. [zlib](https://zlib.net/) (a dependency of libcurl, not this driver)
    * [zlib License](https://www.zlib.net/zlib_license.html)

### Specification Dependencies

1. [ODBC 3.8 Spec](https://learn.microsoft.com/en-us/sql/odbc/reference/what-s-new-in-odbc-3-8)
    * [MIT License](https://github.com/microsoft/ODBC-Specification/blob/master/license.txt)
    * Note: This is not distributed in source form with the TrinoODBC source
      nor in binary form with the installer. This license is mentioned for the
      purpose of transparency, not due to a legal requirement.

### Installer Dependencies

1. [WiX](https://wixtoolset.org/)
    * [Microsoft Reciprocal License (MS-RL)](https://wixtoolset.org/docs/about/)
    * Note: This license does not apply to the TrinoODBC driver because it is
      used soely to bundle/package the driver into an installer package. WiX
      itself is used in unmodified form.

### Testing Dependencies

1. [googletest (aka gtest)](https://github.com/google/googletest)
    * [BSD 3-Clause "Revised" License](https://github.com/google/googletest/blob/main/LICENSE)
    * Note: This is not distributed in source form with the TrinoODBC source
      nor in binary form with the installer. It is required to be provided
      by vcpkg if you wish to compile the test suite for this repository.
      This license is mentioned for the purpose of transparency, not due
      to a legal requirement.

## Development Resources

A treasure trove of API documentation for the ODBC API is available
[here](https://learn.microsoft.com/en-us/sql/odbc/reference/syntax/odbc-api-reference?view=sql-server-ver16).

## Acknowledgements

Initial development of this driver was supported by
[Corteva Agriscience](https://www.corteva.com)
and released as open source under the Apache License 2.0.
We gratefully acknowledge their contribution to the Trino community.
