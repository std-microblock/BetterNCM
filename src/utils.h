#pragma once
#include "pch.h"
#include <string>
#include "pystring/pystring.h"

using namespace std;

string read_to_string(const string& path);
string ws2s(const wstring& str);
bool check_legal_file_path(const string& path);
std::wstring s2ws(const std::string& s, bool isUtf8 = true);
void write_file_text(const string& path, const string& text, bool append = false);

extern string datapath;