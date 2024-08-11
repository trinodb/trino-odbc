#pragma once

#include <map>
#include <string>


/*
Given a set of key-value-pairs encoded in a null-delimited char array,
parse them into a map data structure.

KVPs themselves are delimited by the equals sign.
Between the KVPs are null chars.
At the end is a back-to-back pair of null chars to signal the end of the array.

ex: hello=world\0foo=bar\0\0

This is the format used by ConfigDSN in the ODBC api to communicate
attributes of a DSN, but the logic is general enough to make this
a utility function.
*/
std::map<std::string, std::string>
parseKVPsFromNullDelimChars(const char* kvpChars);

/*
Given a set of key-value-pairs encoded in a semicolon-delimited char array,
parse them into a map data structure. Turn all keys to lowercase in the
process.

KVPs themselves are delimited by the equals sign.
Between the KVPs are semicolons.
At the end there may be a semicolon, and there will be a null char to terminate
the entire string.

ex 1: hello=world;foo=bar;
ex 2: hello=world;foo=bar

This is the format used by connection strings passed into SQLDriverConnect.
*/
std::map<std::string, std::string>
parseKVPsFromSemicolonDelimStr(std::string kvpStr);


/*
Given a set of key-value pairs encoded in a comma delimited string, parse them
into a map data structure. Remove any double-quotes from the keys and values in
the process.

KVPs themselves are delimited by the equals sign.
Between the KVPs are commas and a single whitespace character.
There is nothing at the end of the kvp string itself.

ex: abc="123", "def"="foo"

This is the format used by the www-authenticate header to return the retirect
server and token server to use to authenticate to Trino
*/
std::map<std::string, std::string>
parseKVPsFromCommaDelimStr(std::string kvpStr);
