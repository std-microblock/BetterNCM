export const betterncmFetch = (
	relPath: string,
	option?: RequestInit & {
		ignoreApiKey?: boolean;
	},
) => {
	if (option) {
		option.headers = option.headers ?? {};
		if (!option.ignoreApiKey)
			option.headers["BETTERNCM_API_KEY"] = BETTERNCM_API_KEY;
	} else {
		option = {
			headers: { BETTERNCM_API_KEY },
		};
	}
	return fetch(BETTERNCM_API_PATH + relPath, option);
};
