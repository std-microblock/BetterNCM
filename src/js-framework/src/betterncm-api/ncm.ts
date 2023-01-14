const cachedFunctionMap: Map<string, Function> = new Map();

// rome-ignore lint/suspicious/noExplicitAny: 函数类型可随意
function callCachedSearchFunction<F extends (...args: any[]) => any>(
	searchFunctionName: string,
	args: Parameters<F>,
): ReturnType<F> {
	if (!cachedFunctionMap.has(searchFunctionName)) {
		const findResult = ncm.findApiFunction(searchFunctionName);
		if (findResult) {
			const [func, funcRoot] = findResult;
			cachedFunctionMap.set(searchFunctionName, func.bind(funcRoot));
		}
	}
	const cachedFunc = cachedFunctionMap.get(searchFunctionName);
	if (cachedFunc) {
		return cachedFunc.apply(null, args);
	} else {
		throw new TypeError(`函数 ${searchFunctionName} 未找到`);
	}
}

export namespace ncm {
	export function findNativeFunction(obj: Object, identifiers: string) {
		for (let key in obj) {
			let flag = true;
			for (
				let _i = 0, identifiers_1 = identifiers;
				_i < identifiers_1.length;
				_i++
			) {
				let identifier = identifiers_1[_i];
				if (!obj[key].toString().includes(identifier)) flag = false;
			}
			if (flag) return key;
		}
	}

	export function openUrl(url: string) {
		channel.call("os.navigateExternal", () => {}, [url]);
	}

	export function getNCMPackageVersion(): string {
		return window?.APP_CONF?.packageVersion || "0000000";
	}

	export function getNCMFullVersion(): string {
		return window?.APP_CONF?.appver || "0.0.0.0";
	}

	export function getNCMVersion(): string {
		const v = getNCMFullVersion();
		return v.substring(0, v.lastIndexOf("."));
	}

	export function getNCMBuild(): number {
		const v = getNCMFullVersion();
		return parseInt(v.substring(v.lastIndexOf(".") + 1));
	}

	export function searchApiFunction(
		nameOrFinder: string | ((func: Function) => boolean),
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
		result: [Function, any, string[]][] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [Function, any, string[]][] {
		if (root === undefined || root === null) {
			return [];
		}
		prevObjects.push(root);
		if (typeof nameOrFinder === "string") {
			if (typeof root[nameOrFinder] === "function") {
				result.push([root[nameOrFinder], root, [...currentPath]]);
			}
		} else {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "function" &&
					nameOrFinder(root[key])
				) {
					result.push([root[key], root, [...currentPath]]);
				}
			}
		}
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "object" &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					currentPath.push(key);
					searchApiFunction(
						nameOrFinder,
						root[key],
						currentPath,
						prevObjects,
						result,
					);
					currentPath.pop();
				}
			}
		}
		prevObjects.pop();
		return result;
	}

	export function searchForData(
		// rome-ignore lint/suspicious/noExplicitAny: 会检测任意值
		finder: (func: any) => boolean,
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
		result: [any, any, string[]][] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [any, any, string[]][] {
		if (root === undefined || root === null) {
			return [];
		}
		prevObjects.push(root);
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					if (typeof root[key] === "object") {
						currentPath.push(key);
						searchApiFunction(
							finder,
							root[key],
							currentPath,
							prevObjects,
							result,
						);
						currentPath.pop();
					} else if (finder(root[key])) {
						result.push([root[key], root, [...currentPath]]);
					}
				}
			}
		}
		prevObjects.pop();
		return result;
	}

	export function findApiFunction(
		nameOrFinder: string | ((func: Function) => boolean),
		// rome-ignore lint/suspicious/noExplicitAny: 根对象可以是任意的
		root: any = window,
		currentPath = ["window"],
		// rome-ignore lint/suspicious/noExplicitAny: 已检索对象可以是任意的
		prevObjects: any[] = [],
		// rome-ignore lint/suspicious/noExplicitAny: 返回该函数的携带对象，方便做 bind 绑定
	): [Function, any, string[]] | null {
		if (root === undefined || root === null) {
			return null;
		}
		prevObjects.push(root);
		if (typeof nameOrFinder === "string") {
			if (typeof root[nameOrFinder] === "function") {
				return [root[nameOrFinder], root, [...currentPath]];
			}
		} else {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "function" &&
					nameOrFinder(root[key])
				) {
					return [root[key], root, [...currentPath]];
				}
			}
		}
		if (currentPath.length < 10) {
			for (const key of Object.keys(root)) {
				if (
					Object.hasOwnProperty.call(root, key) &&
					typeof root[key] === "object" &&
					!prevObjects.includes(root[key]) &&
					!(
						currentPath.length === 1 &&
						prevObjects[prevObjects.length - 1] === window &&
						key === "betterncm"
					) // 咱们自己的函数就不需要检测了
				) {
					currentPath.push(key);
					const result = findApiFunction(
						nameOrFinder,
						root[key],
						currentPath,
						prevObjects,
					);
					currentPath.pop();
					if (result) {
						return result;
					}
				}
			}
		}
		prevObjects.pop();
		return null;
	}

	// rome-ignore lint/suspicious/noExplicitAny: 这个是网易云自己暴露的对象，里头有很多可以利用的函数
	declare const dc: any;
	// rome-ignore lint/suspicious/noExplicitAny: 这个是网易云自己暴露的对象，里头有很多可以利用的函数
	declare const ctl: any;

	let cachedGetPlayingFunc: Function | null = null;
	/**
	 * 获取当前正在播放的歌曲的信息，包括歌曲信息，来源，当前播放状态等
	 * @todo 补全返回值类型
	 * @returns 当前歌曲的播放信息
	 */
	export function getPlayingSong() {
		if (cachedGetPlayingFunc === null) {
			const findResult = findApiFunction("getPlaying");
			if (findResult) {
				const [getPlaying, getPlayingRoot] = findResult;
				cachedGetPlayingFunc = getPlaying.bind(getPlayingRoot);
			}
		}
		if (cachedGetPlayingFunc === null) {
			return null;
		} else {
			return cachedGetPlayingFunc();
		}
	}

	/**
	 * 获取当前正在播放的歌曲的简要信息
	 * @deprecated 由于找到了自带的接口，故这个函数被弃用，请转而使用 `betterncm.ncm.getPlayingSong`
	 * @returns 简化的播放信息
	 */
	export function getPlaying() {
		const playing = getPlayingSong();
		const result = {
			id: playing.data.id as number,
			title: playing.data.name as string,
			type: "normal",
		};
		if (playing.from.fm) {
			result.type = "fm";
		}
		return result;
	}
}
