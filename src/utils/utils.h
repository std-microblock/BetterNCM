#pragma once
#include "pch.h"
#include <string>
#define BNSTRING_USE_CEFSTRING_FEATURES
#include "../utils/BNString.hpp"
#include "pystring/pystring.h"
#include <include/internal/cef_string.h>


namespace util {
	BNString read_to_string(const std::filesystem::path& path);
	std::string ws2s(const std::wstring& str);
	std::wstring s2ws(const std::string& s, bool isUtf8 = true);
	void write_file_text_utf8(const std::string& path, const std::string& text, bool append = false);
	void write_file_text(const BNString& path, const BNString& text, bool append = false);
	BNString getEnvironment(const BNString& key);
	BNString getNCMPath();

	BNString get_command_line();

	template<typename S>
	CefString cefFromCEFUserFreeTakeOwnership(S* s) {
		CefString st;
		st.AttachToUserFree(s);
		return st;
	}
	template<typename S>
	CefString cefFromCEFUserFree(S* s) {
		return CefString(s);
	}

	class ScreenCapturePart
	{
	public:
		ScreenCapturePart();
		~ScreenCapturePart();
		char* getData();
		DWORD getDataSize();

	private:
		HDC hdcSource;
		HDC hdcMemory;
		HBITMAP hBitmap;
		HBITMAP hBitmapOld;
		DWORD dwSizeofDIB;
		char* lpbitmap = nullptr;
		char* allData = nullptr;
	};


	// https://stackoverflow.com/questions/1394053/how-to-write-a-generic-alert-message-using-win32
	void alert(const wchar_t* item);
	void alert(const std::wstring* item);
	template<typename T>
	void alert(T item)
	{
		//this accepts all types that supports operator << 
		std::ostringstream os;
		os << item;
		MessageBoxA(NULL, os.str().c_str(), "BetterNCM", MB_OK | MB_ICONINFORMATION);
	}


	std::string random_string(std::string::size_type length);

	std::string guessMimeType(std::string fileExtension);

	std::string load_string_resource(LPCTSTR name);
	std::string wstring_to_utf8(const std::wstring& str);
	std::wstring utf8_to_wstring(const std::string& utf8);
	semver::version getNCMExecutableVersion();
	std::wstring wreplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to);
	void restartNCM();
	void exec(std::wstring cmd, bool ele, bool showWindow = false);
	void killNCM();
	void watchDir(const BNString& directory, std::function<bool(BNString, BNString)>);
}