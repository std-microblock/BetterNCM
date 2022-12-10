/** 一个由 C++ 侧设置的访问密钥，以免出现非法调用 */
declare const BETTERNCM_API_KEY: string;

const BETTERNCM_API_PATH = "http://localhost:3248/api";
const BETTERNCM_FILES_PATH = "http://localhost:3248/local";
export const ncmFetch = (relPath: string, option?: RequestInit) => {
	if (option) {
		option.headers = option.headers || {};
		option.headers["BETTERNCM_API_KEY"] = BETTERNCM_API_KEY;
		return fetch(BETTERNCM_API_PATH + relPath, option);
	} else {
		return fetch(BETTERNCM_API_PATH + relPath, {
			headers: { BETTERNCM_API_PATH },
		});
	}
};
