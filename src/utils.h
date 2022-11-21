#pragma once
#include "pch.h"
#include <string>
#include "pystring/pystring.h"

using namespace std;

string read_to_string(const string& path);
string ws2s(const wstring& str);
std::wstring s2ws(const std::string& s, bool isUtf8 = true);
void write_file_text(const string& path, const string& text, bool append = false);
string getEnvironment(const string& key);
string getNCMPath();

string get_command_line();

extern string datapath;

bool screenCapturePart(LPCWSTR fname);
bool saveBitmap(LPCWSTR filename, HBITMAP bmp, HPALETTE pal);

std::string load_string_resource(LPCTSTR name);