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

template<typename T>
void alert(T item)
{
	//this accepts all types that supports operator << 
	std::ostringstream os;
	os << item;
	MessageBoxA(NULL, os.str().c_str(), "Message", MB_OK | MB_ICONINFORMATION);
}


std::string load_string_resource(LPCTSTR name);
std::string wstring_to_utf_8(const std::wstring& str);
std::wstring utf8_to_wstring(const std::string& utf8);