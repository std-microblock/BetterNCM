#include "pch.h"
#include <ole2.h>
#include <olectl.h>
#include "utils.h"
#include <assert.h>
#pragma comment(lib, "version.lib")

extern HMODULE g_hModule;
string read_to_string(const string& path) {
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return string(buffer.str());
}

// https://stackoverflow.com/questions/4804298/how-to-convert-wstring-into-string   (modified)
string ws2s(wstring const& str)
{
	std::string strTo;
	char* szTo = new char[str.length() + 1];
	szTo[str.size()] = '\0';
	WideCharToMultiByte(CP_ACP, 0, str.c_str(), -1, szTo, (int)str.length(), NULL, NULL);
	strTo = szTo;
	delete szTo;
	return strTo;
}

std::wstring s2ws(const std::string& s, bool isUtf8)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength, 0, 0);
	std::wstring buf;
	buf.resize(len);
	MultiByteToWideChar(isUtf8 ? CP_UTF8 : CP_ACP, 0, s.c_str(), slength,
		const_cast<wchar_t*>(buf.c_str()), len);
	return buf;
}


void write_file_text(const string& path, const string& text, bool append) {
	ofstream file;
	if (append)
		file.open(path, std::ios_base::app);
	else
		file.open(path);

	file << text;
	file.close();
}

// https://stackoverflow.com/questions/4130180/how-to-use-vs-c-getenvironmentvariable-as-cleanly-as-possible
string getEnvironment(const string& key) {
	return ws2s(wstring(_wgetenv(s2ws(key).c_str())));
}

string datapath = "\\betterncm";

wstring getNCMPath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");

	return std::wstring(buffer).substr(0, pos);
}

string get_command_line() {
	LPTSTR cmd = GetCommandLine();

	return ws2s(wstring(cmd));
}


// https://stackoverflow.com/questions/9524393/how-to-capture-part-of-the-screen-and-save-it-to-a-bmp
bool screenCapturePart(LPCWSTR fname) {
	HDC hdcSource = GetDC(NULL);
	HDC hdcMemory = CreateCompatibleDC(hdcSource);

	int capX = GetDeviceCaps(hdcSource, HORZRES);
	int capY = GetDeviceCaps(hdcSource, VERTRES);

	int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
	int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
	int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, w, h);
	HBITMAP hBitmapOld = (HBITMAP)SelectObject(hdcMemory, hBitmap);

	BitBlt(hdcMemory, 0, 0, w, h, hdcSource, x, y, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hdcMemory, hBitmapOld);

	DeleteDC(hdcSource);
	DeleteDC(hdcMemory);

	HPALETTE hpal = NULL;
	if (saveBitmap(fname, hBitmap, hpal)) return true;
	return false;
}

bool saveBitmap(LPCWSTR filename, HBITMAP bmp, HPALETTE pal)
{
	bool result = false;
	PICTDESC pd;

	pd.cbSizeofstruct = sizeof(PICTDESC);
	pd.picType = PICTYPE_BITMAP;
	pd.bmp.hbitmap = bmp;
	pd.bmp.hpal = pal;

	LPPICTURE picture;
	HRESULT res = OleCreatePictureIndirect(&pd, IID_IPicture, false,
		reinterpret_cast<void**>(&picture));

	if (!SUCCEEDED(res))
		return false;

	LPSTREAM stream;
	res = CreateStreamOnHGlobal(0, true, &stream);

	if (!SUCCEEDED(res))
	{
		picture->Release();
		return false;
	}

	LONG bytes_streamed;
	res = picture->SaveAsFile(stream, true, &bytes_streamed);

	HANDLE file = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_READ, 0,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (!SUCCEEDED(res) || !file)
	{
		stream->Release();
		picture->Release();
		return false;
	}

	HGLOBAL mem = 0;
	GetHGlobalFromStream(stream, &mem);
	LPVOID data = GlobalLock(mem);

	DWORD bytes_written;

	result = !!WriteFile(file, data, bytes_streamed, &bytes_written, 0);
	result &= (bytes_written == static_cast<DWORD>(bytes_streamed));

	GlobalUnlock(mem);
	CloseHandle(file);

	stream->Release();
	picture->Release();

	return result;
}

std::string load_string_resource(LPCTSTR name)
{
	HRSRC hRes = FindResource(g_hModule, name, RT_RCDATA);
	assert(hRes);
	DWORD size = SizeofResource(g_hModule, hRes);
	HGLOBAL hGlobal = LoadResource(g_hModule, hRes);
	assert(hGlobal);

	std::string ret;

	const uint8_t bom[3] = { 0xEF, 0xBB, 0xBF };
	uint8_t* ptr = (uint8_t*)LockResource(hGlobal);

	if (size >= 3 && memcmp(bom, ptr, 3) == 0)
	{
		ret.assign((char*)ptr + 3, (size_t)size - 3);
	}
	else
	{
		ret.assign((char*)ptr, (size_t)size);
	}

	UnlockResource(ptr);
	return ret;
}

std::string wstring_to_utf_8(const std::wstring& str)
{
	std::string ret;
	int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
	if (len > 0)
	{
		ret.resize(len);
		WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &ret[0], len, NULL, NULL);
	}
	return ret;
}

// https://stackoverflow.com/questions/7153935/how-to-convert-utf-8-stdstring-to-utf-16-stdwstring
std::wstring utf8_to_wstring(const std::string& utf8)
{
	std::vector<unsigned long> unicode;
	size_t i = 0;
	while (i < utf8.size())
	{
		unsigned long uni;
		size_t todo;
		bool error = false;
		unsigned char ch = utf8[i++];
		if (ch <= 0x7F)
		{
			uni = ch;
			todo = 0;
		}
		else if (ch <= 0xBF)
		{
			throw std::logic_error("not a UTF-8 string");
		}
		else if (ch <= 0xDF)
		{
			uni = ch & 0x1F;
			todo = 1;
		}
		else if (ch <= 0xEF)
		{
			uni = ch & 0x0F;
			todo = 2;
		}
		else if (ch <= 0xF7)
		{
			uni = ch & 0x07;
			todo = 3;
		}
		else
		{
			throw std::logic_error("not a UTF-8 string");
		}
		for (size_t j = 0; j < todo; ++j)
		{
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
	for (size_t i = 0; i < unicode.size(); ++i)
	{
		unsigned long uni = unicode[i];
		if (uni <= 0xFFFF)
		{
			utf16 += (wchar_t)uni;
		}
		else
		{
			uni -= 0x10000;
			utf16 += (wchar_t)((uni >> 10) + 0xD800);
			utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
		}
	}
	return utf16;
}

semver::version getNCMExecutableVersion() {
	DWORD  verHandle = 0;
	UINT   size = 0;
	LPBYTE lpBuffer = NULL;
	DWORD  verSize = GetFileVersionInfoSize((getNCMPath() + L"\\cloudmusic.exe").c_str(), &verHandle);

	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];

		if (GetFileVersionInfo((getNCMPath() + L"\\cloudmusic.exe").c_str(), verHandle, verSize, verData))
		{
			if (VerQueryValue(verData, L"\\", (VOID FAR * FAR*) & lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO* verInfo = (VS_FIXEDFILEINFO*)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
						return semver::version{
							(uint8_t)((verInfo->dwFileVersionMS >> 16) & 0xffff),
							(uint8_t)((verInfo->dwFileVersionMS >> 0) & 0xffff),
							(uint8_t)((verInfo->dwFileVersionLS >> 16) & 0xffff)
						};
					}
				}
			}
		}
		delete[] verData;
	}
}

std::wstring wreplaceAll(std::wstring str, const std::wstring& from, const std::wstring& to) {
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::wstring::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}
	return str;
}

void alert(const wchar_t* item)
{
	MessageBoxW(NULL, item, L"BetterNCM", MB_OK | MB_ICONINFORMATION);
}

void alert(const wstring* item)
{
	MessageBoxW(NULL, item->c_str(), L"BetterNCM", MB_OK | MB_ICONINFORMATION);
}