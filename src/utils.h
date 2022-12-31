#pragma once
#include "pch.h"
#include <string>
#include "../utils/BNString.hpp"
#include "pystring/pystring.h"

BNString read_to_string(const BNString& path);
string ws2s(const wstring& str);
std::wstring s2ws(const string& s, bool isUtf8 = true);
void write_file_text(const BNString& path, const BNString& text, bool append = false);
BNString getEnvironment(const BNString& key);
BNString getNCMPath();

string get_command_line();

extern BNString datapath;

bool screenCapturePart(LPCWSTR fname);
bool saveBitmap(LPCWSTR filename, HBITMAP bmp, HPALETTE pal);

// https://stackoverflow.com/questions/1394053/how-to-write-a-generic-alert-message-using-win32
void alert(const wchar_t* item);
void alert(const wstring* item);
template<typename T>
void alert(T item)
{
	//this accepts all types that supports operator << 
	std::ostringstream os;
	os << item;
	MessageBoxA(NULL, os.str().c_str(), "BetterNCM", MB_OK | MB_ICONINFORMATION);
}


std::string load_string_resource(LPCTSTR name);
std::string wstring_to_utf8(const std::wstring& str);
std::wstring utf8_to_wstring(const std::string& utf8);
semver::version getNCMExecutableVersion();
std::wstring wreplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to);

void restartNCM();