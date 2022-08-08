#include "pch.h"
#include "pystring/pystring.h"
#include "EasyCEFHooks.h"



#pragma comment(linker, "/EXPORT:vSetDdrawflag=_AheadLib_vSetDdrawflag,@1")
#pragma comment(linker, "/EXPORT:AlphaBlend=_AheadLib_AlphaBlend,@2")
#pragma comment(linker, "/EXPORT:DllInitialize=_AheadLib_DllInitialize,@3")
#pragma comment(linker, "/EXPORT:GradientFill=_AheadLib_GradientFill,@4")
#pragma comment(linker, "/EXPORT:TransparentBlt=_AheadLib_TransparentBlt,@5")

#define EXTERNC extern "C"
#define NAKED __declspec(naked)
#define EXPORT __declspec(dllexport)

#define ALCPP EXPORT NAKED
#define ALSTD EXTERNC EXPORT NAKED void __stdcall
#define ALCFAST EXTERNC EXPORT NAKED void __fastcall
#define ALCDECL EXTERNC NAKED void __cdecl
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace std;





string script;



std::wstring s2ws(const std::string& str)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.from_bytes(str);
}

std::string ws2s(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

void message(string title, string text) {
	MessageBox(NULL, s2ws(text).c_str(), s2ws(title).c_str(), 0);
}

void warn(string text) {
	message("BetterNCM 警告", text);
}

BOOL APIENTRY InstallHook()
{
	createBetterNCMFiles();

}




namespace AheadLib
{
	HMODULE m_hModule = NULL;	// ԭʼģ����
	DWORD m_dwReturn[5] = { 0 };	// ԭʼ�������ص�ַ


	// ����ԭʼģ��
	inline BOOL WINAPI Load()
	{
		TCHAR tzPath[MAX_PATH];
		TCHAR tzTemp[MAX_PATH * 2];

		GetSystemDirectory(tzPath, MAX_PATH);
		lstrcat(tzPath, TEXT("\\msimg32"));
		m_hModule = LoadLibrary(tzPath);
		if (m_hModule == NULL)
		{
			wsprintf(tzTemp, TEXT("�޷����� %s�������޷��������С�"), tzPath);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
		}

		return (m_hModule != NULL);
	}

	// �ͷ�ԭʼģ��
	inline VOID WINAPI Free()
	{
		if (m_hModule)
		{
			FreeLibrary(m_hModule);
		}
	}

	// ��ȡԭʼ������ַ
	FARPROC WINAPI GetAddress(PCSTR pszProcName)
	{
		FARPROC fpAddress;
		CHAR szProcName[16];
		TCHAR tzTemp[MAX_PATH];

		fpAddress = GetProcAddress(m_hModule, pszProcName);
		if (fpAddress == NULL)
		{
			if (HIWORD(pszProcName) == 0)
			{
				//printf(szProcName, "%d", pszProcName);
				pszProcName = szProcName;
			}

			wsprintf(tzTemp, TEXT("�޷��ҵ����� %hs�������޷��������С�"), pszProcName);
			MessageBox(NULL, tzTemp, TEXT("AheadLib"), MB_ICONSTOP);
			ExitProcess(-2);
		}

		return fpAddress;
	}
}
using namespace AheadLib;
thread* servert;

string read_to_string(string path) {
	std::ifstream t(path);
	std::stringstream buffer;
	buffer << t.rdbuf();
	return string(buffer.str());
}

string datapath = string(getenv("USERPROFILE")) + "\\betterncm";

bool check_legal_file_path(string path) {
	return pystring::find(path, "..") == -1;
}

// https://blog.insane.engineer/post/cpp_read_file_into_string/
std::string file_contents(const std::filesystem::path& path)
{
	// Sanity check
	if (!std::filesystem::is_regular_file(path))
		return { };

	// Open the file
	// Note that we have to use binary mode as we want to return a string
	// representing matching the bytes of the file on the file system.
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
		return { };

	// Read contents
	std::string content{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };

	// Close the file
	file.close();

	return content;
}

// https://stackoverflow.com/questions/51352863/what-is-the-idiomatic-c17-standard-approach-to-reading-binary-files
std::vector<std::byte> load_file(std::string const& filepath)
{
	std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);

	if (!ifs)
		throw std::runtime_error(filepath + ": " + std::strerror(errno));

	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);

	auto size = std::size_t(end - ifs.tellg());

	if (size == 0) // avoid undefined behavior 
		return {};

	std::vector<std::byte> buffer(size);

	if (!ifs.read((char*)buffer.data(), buffer.size()))
		throw std::runtime_error(filepath + ": " + std::strerror(errno));

	return buffer;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, PVOID pvReserved)
{
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		servert = new thread([=] {

			httplib::Server svr;
			svr.Get("/api/fs/read_dir", [&](const httplib::Request& req, httplib::Response& res) {
				using namespace std;
				namespace fs = std::filesystem;

				auto path = req.get_param_value("path");

				vector<string> paths;

				if (check_legal_file_path(path)) {
					for (const auto& entry : fs::directory_iterator(datapath+"/"+path))
						paths.push_back(pystring::slice(entry.path().string(),datapath.length()));
				}
				else {
					paths.push_back("Error: Access Denied");
					res.status = 400;
				}

				res.set_content(((nlohmann::json)paths).dump(), "application/json");
			});

			svr.Get("/api/fs/read_file_text", [&](const httplib::Request& req, httplib::Response& res) {
				using namespace std;
				namespace fs = std::filesystem;

				auto path = req.get_param_value("path");

				if (check_legal_file_path(path)) {
					res.set_content(file_contents(datapath + "/" + path), "text/plain");
				}
				else {
					res.set_content("Error: Access Denied","text/plain");
					res.status = 400;
				}
			});

			svr.Post("/api/fs/write_file_text", [&](const httplib::Request& req, httplib::Response& res) {
				using namespace std;
				namespace fs = std::filesystem;

				auto path = req.get_param_value("path");

				if (check_legal_file_path(path)) {
					ofstream file;
					file.open(datapath + "/" + path);
					file << req.body;
					file.close();
					res.status = 200;
				}
				else {
					res.set_content("Error: Access Denied", "text/plain");
					res.status = 400;
				}
			});

			svr.set_mount_point("/local", datapath);

			//svr.Get("/read_file", [&](const httplib::Request& req, httplib::Response& res) {
			//	string filename = req.get_param_value("filename");
			//	if (filename.find_first_of("..") != string::npos) {
			//		res.status = 502;
			//		res.set_content("Bad request","");
			//	}
			//	res.set_content(read_to_string(datapath + "/" + filename), "application/json");
			//}); 


			//
			//svr.Get("/explorer", [&](const httplib::Request& req, httplib::Response& res) {
			//	auto command = string("cmd /c explorer.exe \"") + datapath + "\"";
			//	system(command.c_str());
			//	res.set_content(command,"");
			//});
			svr.listen("0.0.0.0", 3248);
			});

		script = R"(
if(!window["__BETTERNCM___"]){
window["__BETTERNCM___"]=true

var addons;
var reloadInject = {}
function main() {
    fetch("http://localhost:3248/addons").then(function (resp) { return resp.text() }).then(function (resp1) {
        addons = JSON.parse(resp1);
        setInterval(function () {
            for (var x = 0; x < addons.length; x++) {
                var addon = addons[x];
                if(addon.enabled)
                if (addon.devMode) {
                    fetch("http://localhost:3248/read_file?filename=" + addon.file)
                        .then(function (resp_2856) { return resp_2856.text() })
                        .then(function (resp2) {
                            if (reloadInject[addon.file] && resp2 != reloadInject[addon.file]) {
                                document.location.reload()
                            }
                        })
                }
            }
        }, 1000);
        for (var x = 0; x < addons.length; x++) {
            var addon = addons[x];
            if(addon.enabled)
            fetch("http://localhost:3248/read_file?filename=" + addon.file)
                .then(function (resp) { return resp.text() })
                .then(function (resp) {
                    reloadInject[addon.file] = resp
                    var script = document.createElement("script");
                    script.innerHTML = resp;
                    document.body.appendChild(script);
                })
		
        }
fetch("http://localhost:3248/injectsucceeded")
    });
}
setTimeout(function(){main()},1000)
}
)";

		EasyCEFHooks::onKeyEvent = [](auto client, auto browser, auto event) {
			if (event->type == KEYEVENT_KEYUP && event->windows_key_code == 123) {


				auto cef_browser_host = browser->get_host(browser);
				CefWindowInfo windowInfo{};
				CefBrowserSettings settings{};
				CefPoint point{};
				windowInfo.SetAsPopup(NULL, "EasyCEFInject DevTools");

				cef_browser_host->show_dev_tools(cef_browser_host, &windowInfo, client, &settings, &point);
			}
		};

		EasyCEFHooks::onLoadStart = [](_cef_browser_t* browser, _cef_frame_t* frame, auto transition_type) {
			if (frame->is_main(frame) && frame->is_valid(frame)) {
				wstring url = frame->get_url(frame)->str;
				EasyCEFHooks::executeJavaScript(frame, string("console.log('yeeeeeee ") + ws2s(url) + "')");
			}
		};

		EasyCEFHooks::InstallHooks();

		DisableThreadLibraryCalls(hModule);

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			m_dwReturn[i] = TlsAlloc();
		}

		return Load();
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{

		for (INT i = 0; i < sizeof(m_dwReturn) / sizeof(DWORD); i++)
		{
			TlsFree(m_dwReturn[i]);
		}

		Free();
		EasyCEFHooks::UninstallHook();
	}

	return TRUE;
}

ALCDECL AheadLib_vSetDdrawflag(void)
{
	// ���淵�ص�ַ�� TLS
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	// ����ԭʼ����
	GetAddress("vSetDdrawflag")();

	// ��ȡ���ص�ַ������
	__asm PUSH EAX;
	__asm PUSH m_dwReturn[0 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
ALCDECL AheadLib_AlphaBlend(void)
{
	// ���淵�ص�ַ�� TLS
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	// ����ԭʼ����
	GetAddress("AlphaBlend")();

	// ��ȡ���ص�ַ������
	__asm PUSH EAX;
	__asm PUSH m_dwReturn[1 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
ALCDECL AheadLib_DllInitialize(void)
{
	// ���淵�ص�ַ�� TLS
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	// ����ԭʼ����
	GetAddress("DllInitialize")();

	// ��ȡ���ص�ַ������
	__asm PUSH EAX;
	__asm PUSH m_dwReturn[2 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
ALCDECL AheadLib_GradientFill(void)
{
	// ���淵�ص�ַ�� TLS
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	// ����ԭʼ����
	GetAddress("GradientFill")();

	// ��ȡ���ص�ַ������
	__asm PUSH EAX;
	__asm PUSH m_dwReturn[3 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ��������
ALCDECL AheadLib_TransparentBlt(void)
{
	// ���淵�ص�ַ�� TLS
	__asm PUSH m_dwReturn[4 * TYPE long];
	__asm CALL DWORD PTR[TlsSetValue];

	// ����ԭʼ����
	GetAddress("TransparentBlt")();

	// ��ȡ���ص�ַ������
	__asm PUSH EAX;
	__asm PUSH m_dwReturn[4 * TYPE long];
	__asm CALL DWORD PTR[TlsGetValue];
	__asm XCHG EAX, [ESP];
	__asm RET;
}