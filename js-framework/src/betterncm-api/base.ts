export const BETTERNCM_API_PATH = "http://localhost:3248/api";
export const BETTERNCM_FILES_PATH = "http://localhost:3248/local";
window.BETTERNCM_API_PATH = BETTERNCM_API_PATH;
window.BETTERNCM_FILES_PATH = BETTERNCM_FILES_PATH;
export const ncmFetch = (relPath: string, option?: RequestInit) => {
	if (option) {
		option.headers = option.headers || {};
		option.headers["BETTERNCM_API_KEY"] = BETTERNCM_API_KEY;
		return fetch(BETTERNCM_API_PATH + relPath, option);
	} else {
		return fetch(BETTERNCM_API_PATH + relPath, {
			headers: { BETTERNCM_API_KEY },
		});
	}
};
