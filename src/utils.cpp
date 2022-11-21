#include "pch.h"
#include <ole2.h>
#include <olectl.h>
#include "utils.h"

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

string datapath ="\\betterncm";

string getNCMPath() {
	wchar_t buffer[MAX_PATH];
	GetModuleFileNameW(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");

	return ws2s(std::wstring(buffer).substr(0, pos));
}

string get_command_line() {
	LPTSTR cmd = GetCommandLine();

	return ws2s(wstring(cmd));
}


// https://stackoverflow.com/questions/9524393/how-to-capture-part-of-the-screen-and-save-it-to-a-bmp
bool screenCapturePart( LPCWSTR fname) {
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

//string read_to_string(const string& path) {
//	std::ifstream t(path);
//	std::stringstream buffer;
//	buffer << t.rdbuf();
//	return string(buffer.str());
//}
////
//string ws2s(const wstring& str) {
//	using convert_typeX = std::codecvt_utf8<wchar_t>;
//	std::wstring_convert<convert_typeX, wchar_t> converterX;
//
//	return converterX.to_bytes(str);
//}
//
//bool check_legal_file_path(const string& path) {
//	return pystring::find(path, "..") == -1;
//}
//
//void write_file_text(const string& path, const string& text) {
//	ofstream file;
//	file.open(path);
//	file << text;
//	file.close();
//}
//
//string datapath = string(getenv("USERPROFILE")) + "\\betterncm"