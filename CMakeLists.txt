# CMakeList.txt : CMake project for TrinoODBC, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.14)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("TrinoODBC")

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

# Define the DEBUG definition so it can be used in preprocessor
# directives to do conditional compilation.
if (CMAKE_BUILD_TYPE MATCHES Debug)
  add_definitions(-DDEBUG)
endif()


find_package(CURL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

# Add source to this project's library
add_library(TrinoODBC SHARED
            "src/trinoAPIWrapper/authProvider/tokens/tokenCache.cpp"
            "src/trinoAPIWrapper/authProvider/tokens/tokenParser.cpp"
            "src/trinoAPIWrapper/authProvider/clientCredAuthProvider.cpp"
            "src/trinoAPIWrapper/authProvider/externalAuthProvider.cpp"
            "src/trinoAPIWrapper/authProvider/noAuthProvider.cpp"
            "src/trinoAPIWrapper/authProvider/tokenCacheAuthProviderBase.cpp"
            "src/trinoAPIWrapper/trinoQuery.cpp"
            "src/trinoAPIWrapper/connectionConfig.cpp"
            "src/trinoAPIWrapper/environmentConfig.cpp"
            "src/trinoAPIWrapper/columnDescription.cpp"
            "src/trinoAPIWrapper/trinoExceptions.cpp"
            "src/driver/config/configDSN.cpp"
            "src/driver/config/driverConfig.cpp"
            "src/driver/config/dsnConfigForm.cpp"
            "src/driver/config/profileReader.cpp"
            "src/driver/config/win32controls/labelMaker.cpp"
            "src/driver/config/win32controls/comboboxMaker.cpp"
            "src/driver/config/win32controls/editMaker.cpp"
            "src/driver/config/win32controls/buttonMaker.cpp"
            "src/driver/handles/envHandle.cpp"
            "src/driver/handles/connHandle.cpp"
            "src/driver/handles/statementHandle.cpp"
            "src/driver/handles/descriptorHandle.cpp"
            "src/driver/handles/handleErrorInfo.cpp"
            "src/driver/mappings/typeMappings.cpp"
            "src/util/browserInteraction.cpp"
            "src/util/b64decoder.cpp"
            "src/util/capitalize.cpp"
            "src/util/cryptUtils.cpp"
            "src/util/dateAndTimeUtils.cpp"
            "src/util/decimalHelper.cpp"
            "src/util/delimKvphelper.cpp"
            "src/util/rowToBuffer.cpp"
            "src/util/stringFromChar.cpp"
            "src/util/stringSplitAndTrim.cpp"
            "src/util/stringTrim.cpp"
            "src/util/timer.cpp"
            "src/util/timeUtils.cpp"
            "src/util/writeLog.cpp"
            "src/driver/allocHandle.cpp"
            "src/driver/bindCol.cpp"
            "src/driver/bindParameter.cpp"
            "src/driver/cancel.cpp"
            "src/driver/cancelHandle.cpp"
            "src/driver/closeCursor.cpp"
            "src/driver/colAttribute.cpp"
            "src/driver/columns.cpp"
            "src/driver/connect.cpp"
            "src/driver/copyDesc.cpp"
            "src/driver/dataSources.cpp"
            "src/driver/describeCol.cpp"
            "src/driver/disconnect.cpp"
            "src/driver/driverConnect.cpp"
            "src/driver/drivers.cpp"
            "src/driver/endTran.cpp"
            "src/driver/execDirect.cpp"
            "src/driver/execute.cpp"
            "src/driver/extendedFetch.cpp"
            "src/driver/fetch.cpp"
            "src/driver/fetchScroll.cpp"
            "src/driver/freeHandle.cpp"
            "src/driver/freeStmt.cpp"
            "src/driver/getConnectAttr.cpp"
            "src/driver/getCursorName.cpp"
            "src/driver/getData.cpp"
            "src/driver/getDescField.cpp"
            "src/driver/getDescRec.cpp"
            "src/driver/getDiagRec.cpp"
            "src/driver/getEnvAttr.cpp"
            "src/driver/getFunctions.cpp"
            "src/driver/getInfo.cpp"
            "src/driver/getStmtAttr.cpp"
            "src/driver/getTypeInfo.cpp"
            "src/driver/moreResults.cpp"
            "src/driver/nativeSql.cpp"
            "src/driver/numParams.cpp"
            "src/driver/numResultCols.cpp"
            "src/driver/paramData.cpp"
            "src/driver/prepare.cpp"
            "src/driver/putData.cpp"
            "src/driver/rowCount.cpp"
            "src/driver/setConnectAttr.cpp"
            "src/driver/setCursorName.cpp"
            "src/driver/setDescField.cpp"
            "src/driver/setDescRec.cpp"
            "src/driver/setEnvAttr.cpp"
            "src/driver/setStmtAttr.cpp"
            "src/driver/statistics.cpp"
            "src/driver/tables.cpp"
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET TrinoODBC PROPERTY CXX_STANDARD 20)
endif()

target_link_libraries(TrinoODBC PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(TrinoODBC PRIVATE CURL::libcurl)
# odbccp32 has linker errors that can be resolved by including the
# legacy_stdio_defnitions library.
# https://learn.microsoft.com/en-us/cpp/error-messages/tool-errors/linker-tools-error-lnk2019
target_link_libraries(TrinoODBC PRIVATE odbccp32 crypt32 legacy_stdio_definitions)

# We want to be able to write unit tests with gtest
enable_testing()
find_package(GTest CONFIG REQUIRED)
add_executable(TestDriver
    "test/connections/connectTest.cpp"
    "test/fixtures/sqlDriverConnectFixture.cpp"
    "test/functions/testCancel.cpp"
    "test/functions/testGetConnectAttr.cpp"
    "test/functions/testGetInfo.cpp"
    "test/functions/testTables.cpp"
    "test/memory/memoryReclamationTest.cpp"
    "test/performance/bindFetchPerformanceTest.cpp"
    "test/performance/fetchRowPerformanceTest.cpp"
    "test/performance/getDataFetchPerformanceTest.cpp"
    "test/types/fetchBindTest.cpp"
    "test/types/fetchGetDataTest.cpp"
    "test/unit/util/base64decoderTest.cpp"
    "test/unit/util/cryptUtilsTest.cpp"
    "test/unit/util/dateAndTimeUtilsTest.cpp"
    "test/unit/util/stringTrimTest.cpp"
    "test/constants.cpp"
    "test/gtestTest.cpp"
)
# GMock is broken in VS environment. Use tests only
# https://github.com/google/googletest/issues/2157
target_link_libraries(TestDriver PRIVATE GTest::gtest GTest::gtest_main odbc32)
target_link_libraries(TestDriver PRIVATE TrinoODBC)
