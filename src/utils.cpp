#include "pch.h"
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
	DWORD bufferSize = 65535; //Limit according to http://msdn.microsoft.com/en-us/library/ms683188.aspx
	std::wstring buff;
	buff.resize(bufferSize);
	bufferSize = GetEnvironmentVariableW(s2ws(key).c_str(), &buff[0], bufferSize);
	if (!bufferSize)
		//error
		buff.resize(bufferSize);

	return ws2s(buff);
}

string datapath = getenv("BETTERNCM_PROFILE")? getEnvironment("BETTERNCM_PROFILE"): getEnvironment("USERPROFILE") + "\\betterncm";


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