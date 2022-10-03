#include "pch.h"
#include "utils.h"

string read_to_string(const string& path) {
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return string(buffer.str());
}

string ws2s(const wstring& str) {
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(str);
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

string datapath = string(getenv("USERPROFILE")) + "\\betterncm";


string get_command_line() {
	LPTSTR cmd = GetCommandLine();

	return ws2s(wstring(cmd));
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