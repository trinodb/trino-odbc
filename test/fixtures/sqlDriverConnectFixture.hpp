#pragma once
#include <windows.h>

#include <sql.h>
#include <sqlext.h>

#include <gtest/gtest.h>

class SQLDriverConnectFixture : public ::testing::Test {
  protected:
    SQLHENV hEnv   = nullptr;
    SQLHDBC hDbc   = nullptr;
    SQLHSTMT hStmt = nullptr;

    void SetUp() override;
    void SetUp(std::string extraConnStr);
    void TearDown() override;

    void maybeReportStatementError(SQLRETURN ret);
    void maybeReportConnectionError(SQLRETURN ret);
};
