#include "utils.h"
#include "pch.h"
#include <ole2.h>
#include <olectl.h>
#include <tlhelp32.h>
#include <shellapi.h>
#include <assert.h>
#pragma comment(lib, "version.lib")
using namespace util;


extern HMODULE g_hModule;

BNString util::read_to_string(const std::filesystem::path& path) {
	std::wifstream file(path);
	std::wstring content((std::istreambuf_iterator<wchar_t>(file)),
		std::istreambuf_iterator<wchar_t>());
	return content;
}

// https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string   (modified)
std::string util::ws2s(const std::wstring& str) {
	std::string strTo;
	auto szTo = new char[str.length() + 1];
	szTo[str.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, szTo, static_cast<int>(str.length()), nullptr, nullptr);
	strTo = szTo;
	delete szTo;
	return strTo;
}

std::wstring util::s2ws(const std::string& s, bool isUtf8) {
	int len;
	int slength = static_cast<int>(s.length()) + 1;
	len = MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength, nullptr, 0);
	std::wstring buf;
	buf.resize(len);
	MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength,
		const_cast<wchar_t*>(buf.c_str()), len);
	return buf;
}

void util::write_file_text_utf8(const std::string& path, const std::string& text, bool append) {
	std::ofstream file;
	if (append)
		file.open(path, std::ios_base::app);
	else
		file.open(path);

	file << text;
	file.close();
}

void util::write_file_text(const BNString& path, const BNString& text, bool append) {
	std::wofstream file;
	if (append)
		file.open(path, std::ios_base::app);
	else
		file.open(path);

	file << text;
	file.close();
}

// https://stackoverflow.com/questions/4130180/how-to-use-vs-c-getenvironmentvariable-as-cleanly-as-possible
BNString util::getEnvironment(const BNString& key) {
	if (!_wgetenv(key.c_str()))return BNString("");
	return std::wstring(_wgetenv(key.c_str()));
}

BNString datapath = "\\betterncm";

BNString util::getNCMPath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(nullptr, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	if (pos != std::wstring::npos)
		buffer[pos] = L'\0';
	return std::wstring(buffer);
}

BNString util::get_command_line() {
	LPTSTR cmd = GetCommandLine();

	return std::wstring(cmd);
}


ScreenCapturePart::ScreenCapturePart() {
	this->hdcSource = GetDC(nullptr);
	this->hdcMemory = CreateCompatibleDC(hdcSource);

	int capX = GetDeviceCaps(hdcSource, HORZRES);
	int capY = GetDeviceCaps(hdcSource, VERTRES);

	int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	this->hBitmap = CreateCompatibleBitmap(hdcSource, w, h);
	this->hBitmapOld = static_cast<HBITMAP>(SelectObject(hdcMemory, this->hBitmap));

	BitBlt(hdcMemory, 0, 0, w, h, hdcSource, x, y, SRCCOPY);
	this->hBitmap = static_cast<HBITMAP>(SelectObject(hdcMemory, this->hBitmapOld));

	BITMAPINFOHEADER bmiHeader{};
	bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmiHeader.biWidth = w;
	bmiHeader.biHeight = h;
	bmiHeader.biPlanes = 1;
	bmiHeader.biBitCount = 24;
	bmiHeader.biCompression = BI_RGB;
	bmiHeader.biSizeImage = 0;
	bmiHeader.biXPelsPerMeter = 0;
	bmiHeader.biYPelsPerMeter = 0;
	bmiHeader.biClrUsed = 0;
	bmiHeader.biClrImportant = 0;

	DWORD dwBmpSize = ((w * bmiHeader.biBitCount + 31) / 32) * 4 * h;
	this->dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	BITMAPFILEHEADER bmfHeader{};
	bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmfHeader.bfSize = this->dwSizeofDIB;
	bmfHeader.bfType = 0x4D42;

	this->lpbitmap = new char[dwBmpSize];
	ZeroMemory(lpbitmap, dwBmpSize);
	GetDIBits(hdcMemory, this->hBitmap, 0, h, lpbitmap, (BITMAPINFO*)&bmiHeader, DIB_RGB_COLORS);

	this->allData = new char[this->dwSizeofDIB];
	ZeroMemory(allData, this->dwSizeofDIB);
	memcpy(allData, &bmfHeader, sizeof(BITMAPFILEHEADER));
	memcpy(allData + sizeof(BITMAPFILEHEADER), &bmiHeader, sizeof(BITMAPINFOHEADER));
	memcpy(allData + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER), lpbitmap, dwBmpSize);
}

ScreenCapturePart::~ScreenCapturePart() {
	ReleaseDC(nullptr, this->hdcSource);
	DeleteDC(this->hdcMemory);
	DeleteObject(this->hBitmap);
	DeleteObject(this->hBitmapOld);
	delete[] this->allData;
	delete[] this->lpbitmap;
	this->allData = nullptr;
	this->lpbitmap = nullptr;
}

char* ScreenCapturePart::getData() {
	return this->allData;
}

// https://stackoverflow.com/questions/440133/how-do-i-create-a-random-alpha-numeric-string-in-c
std::string util::random_string(std::string::size_type length) {
	static auto& chrs = "0123456789"
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	thread_local static std::mt19937 rg{ std::random_device{}() };
	thread_local static std::uniform_int_distribution<std::string::size_type> pick(0, sizeof(chrs) - 2);

	std::string s;

	s.reserve(length);

	while (length--)
		s += chrs[pick(rg)];

	return s;
}

DWORD ScreenCapturePart::getDataSize() {
	return this->dwSizeofDIB;
}

std::map<std::string, std::string> mimeTypes = {
		{".html", "text/html"},
		{".txt", "text/plain"},
		{".jpg", "image/jpeg"},
		{".jpeg", "image/jpeg"},
		{".png", "image/png"},
		{".gif", "image/gif"},
		{".css", "text/css"},
		{".js", "application/javascript"},
		{".flac", "audio/mpeg"},
		{".mp3", "audio/mpeg"}
};

std::string util::guessMimeType(std::string fileExtension) {
	std::string mimeType = "application/octet-stream";

	if (mimeTypes.contains(fileExtension)) {
		mimeType = mimeTypes[fileExtension];
	}

	return mimeType;
}

std::string util::load_string_resource(LPCTSTR name) {
	HRSRC hRes = FindResource(g_hModule, name, RT_RCDATA);
	assert(hRes);
	DWORD size = SizeofResource(g_hModule, hRes);
	HGLOBAL hGlobal = LoadResource(g_hModule, hRes);
	assert(hGlobal);

	std::string ret;

	const uint8_t bom[3] = { 0xEF, 0xBB, 0xBF };
	auto ptr = static_cast<uint8_t*>(LockResource(hGlobal));

	if (size >= 3 && memcmp(bom, ptr, 3) == 0) {
		ret.assign((char*)ptr + 3, static_cast<size_t>(size) - 3);
	}
	else {
		ret.assign((char*)ptr, static_cast<size_t>(size));
	}

	UnlockResource(ptr);
	return ret;
}

std::string util::wstring_to_utf8(const std::wstring& str) {
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), nullptr, 0, nullptr, nullptr);
	if (len > 0) {
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len, nullptr, nullptr);
	}
	return ret;
}

// https://stackoverflow.com/questions/7153935/how-to-convert-utf-8-stdstring-to-utf-16-stdwstring
std::wstring util::utf8_to_wstring(const std::string& utf8) {
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size()) {
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F) {
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF) {
			throw std::logic_error("not a UTF-8 string");
		}
		else if (ch <= 0xDF) {
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF) {
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7) {
			uni = ch & 0x07;
			todo = 3;
		}
		else {
			throw std::logic_error("not a UTF-8 string");
		}
		for (size_t j = 0; j < todo; ++j) {
			if (i == utf8.size())
				throw std::logic_error("not a UTF-8 string");
			unsigned char ch = utf8[i++];
			if (ch < 0x80 || ch > 0xBF)
				throw std::logic_error("not a UTF-8 string");
			uni <<= 6;
			uni += ch & 0x3F;
		}
		if (uni >= 0xD800 && uni <= 0xDFFF)
			throw std::logic_error("not a UTF-8 string");
		if (uni > 0x10FFFF)
			throw std::logic_error("not a UTF-8 string");
		unicode.push_back(uni);
	}
	std::wstring utf16;
	for (size_t i = 0; i < unicode.size(); ++i) {
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF) {
			utf16 += static_cast<wchar_t>(uni);
		}
		else {
			uni -= 0x10000;
			utf16 += static_cast<wchar_t>((uni >> 10) + 0xD800);
			utf16 += static_cast<wchar_t>((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}

semver::version util::getNCMExecutableVersion() {
	DWORD verHandle = 0;
	UINT size = 0;
	LPBYTE lpBuffer = nullptr;
	DWORD verSize = GetFileVersionInfoSize((getNCMPath() + L"\\cloudmusic.exe").c_str(), &verHandle);

	if (verSize != NULL) {
		auto verData = new char[verSize];

		if (GetFileVersionInfo((getNCMPath() + L"\\cloudmusic.exe").c_str(), verHandle, verSize, verData)) {
			if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size)) {
				if (size) {
					auto verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd) {
						return semver::version{
								static_cast<uint8_t>((verInfo->dwFileVersionMS >> 16) & 0xffff),
								static_cast<uint8_t>((verInfo->dwFileVersionMS >> 0) & 0xffff),
								static_cast<uint8_t>((verInfo->dwFileVersionLS >> 16) & 0xffff)
						};
					}
				}
			}
		}
		delete[] verData;
	}
}

std::wstring util::wreplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void util::killNCM() {
	// Get the ID of the current process
	DWORD dwCurrentProcessId = GetCurrentProcessId();

	// Get a snapshot of all the processes in the system
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		return;
	}
	std::vector<DWORD> pidlist;
	// Set up the process entry structure
	PROCESSENTRY32W processEntry;
	processEntry.dwSize = sizeof(PROCESSENTRY32W);

	// Iterate through the processes in the snapshot
	if (Process32FirstW(hSnapshot, &processEntry)) {
		do {
			if (wcscmp(processEntry.szExeFile, L"cloudmusic.exe") == 0) {
				pidlist.push_back(processEntry.th32ProcessID);
			}
		} while (Process32NextW(hSnapshot, &processEntry));
	}

	// Close the snapshot handle
	CloseHandle(hSnapshot);

	std::string cmd = "cmd /c echo";
	for (const auto& pid : pidlist) {
		cmd += " & taskkill /f /pid ";
		cmd += std::to_string(pid);
	}
	exec(s2ws(cmd), false);
}

void util::watchDir(const BNString& directory, std::function<bool(BNString, BNString)> callback) {
	HANDLE hDirectory = CreateFileW(directory.c_str(),
		FILE_LIST_DIRECTORY,
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		nullptr,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
		nullptr);

	if (hDirectory == INVALID_HANDLE_VALUE) {
		std::wcerr << L"Error opening directory: " << GetLastError() << std::endl;
		return;
	}

	OVERLAPPED overlapped = { 0 };
	HANDLE hEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
	overlapped.hEvent = hEvent;

	char buffer[4096];

	while (true) {
		if (ReadDirectoryChangesW(hDirectory,
			buffer,
			sizeof(buffer),
			TRUE,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
			FILE_NOTIFY_CHANGE_ATTRIBUTES |
			FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE,
			nullptr,
			&overlapped,
			nullptr)) {
			WaitForSingleObject(hEvent, INFINITE);

			DWORD dwBytes;
			if (!GetOverlappedResult(hDirectory, &overlapped, &dwBytes, FALSE)) {
				std::wcerr << L"Error getting overlapped result: " << GetLastError() << std::endl;
				break;
			}

			auto pNotify = (PFILE_NOTIFY_INFORMATION)buffer;
			while (pNotify != nullptr) {
				std::wstring fileName(pNotify->FileName, pNotify->FileNameLength / sizeof(WCHAR));
				if (!callback(directory, fileName))goto close;
				if (pNotify->NextEntryOffset == 0) {
					pNotify = nullptr;
				}
				else {
					pNotify = (PFILE_NOTIFY_INFORMATION)(((LPBYTE)pNotify) + pNotify->NextEntryOffset);
				}
			}

			ResetEvent(hEvent);
		}
		else {
			std::wcerr << L"Error reading directory changes: " << GetLastError() << std::endl;
			break;
		}
	}
close:
	CloseHandle(hDirectory);
	CloseHandle(hEvent);
}

bool util::DownloadFile(const BNString& url, const BNString& dest)
{
	// Initialize WinInet library
	HINTERNET hInternet = InternetOpenW(L"Download", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!hInternet)
	{
		return FALSE;
	}

	// Open the URL
	HINTERNET hUrl = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hUrl)
	{
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	// Create a file at the destination path
	HANDLE hFile = CreateFileW(
		dest.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInternet);
		return FALSE;
	}

	// Download the file and write to the created file
	DWORD dwBytesRead = 0;
	BOOL bResult = FALSE;
	CHAR szBuffer[4096];
	while (InternetReadFile(hUrl, szBuffer, 4096, &dwBytesRead) && dwBytesRead)
	{
		DWORD dwBytesWritten = 0;
		bResult = WriteFile(hFile, szBuffer, dwBytesRead, &dwBytesWritten, NULL);
		if (!bResult)
		{
			break;
		}
	}

	// Clean up
	CloseHandle(hFile);
	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	return bResult;
}

BNString util::FetchWebContent(const BNString& url)
{
	HINTERNET hInternet = InternetOpenW(L"WinINetExample", INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (hInternet == NULL) {
		return "";
	}

	HINTERNET hUrl = InternetOpenUrlW(hInternet, url.c_str(), NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (hUrl == NULL) {
		InternetCloseHandle(hInternet);
		return "";
	}

	std::string content;

	DWORD bytesRead = 0;
	char buffer[1024];
	while (InternetReadFile(hUrl, buffer, sizeof(buffer), &bytesRead) && bytesRead != 0) {
		std::string temp(buffer, bytesRead);
		content += std::string(temp.begin(), temp.end());
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInternet);

	return content;
}

void util::restartNCM() {
	WCHAR szProcessName[MAX_PATH];
	GetModuleFileNameW(nullptr, szProcessName, MAX_PATH);
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	killNCM();
	CreateProcessW(szProcessName, nullptr, nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
}


void util::alert(const wchar_t* item) {
	MessageBoxW(nullptr, item, L"BetterNCM", MB_OK | MB_ICONINFORMATION);
}

void util::alert(const std::wstring* item) {
	MessageBoxW(nullptr, item->c_str(), L"BetterNCM", MB_OK | MB_ICONINFORMATION);
}


void util::exec(std::wstring cmd, bool ele, bool showWindow) {
	int nArg;
	LPWSTR* pArgs = CommandLineToArgvW(cmd.c_str(), &nArg);
	if (nArg > 0) {
		std::wstring param;
		SHELLEXECUTEINFOW info;
		ZeroMemory(&info, sizeof(info));
		info.cbSize = sizeof(info);
		info.fMask = 0;
		info.hwnd = nullptr;
		info.lpVerb = ele ? L"runas" : L"open";

		info.lpFile = pArgs[0];

		if (nArg >= 2) {
			for (int i = 1; i < nArg; ++i) {
				if (i > 1) param += L' ';
				param += pArgs[i];
			}
			info.lpParameters = param.c_str();
		}
		else {
			info.lpParameters = nullptr;
		}
		info.lpDirectory = nullptr;
		info.nShow = showWindow ? SW_SHOW : SW_HIDE;

		ShellExecuteExW(&info);
	}

	LocalFree(pArgs);
}
