export namespace utils {
	type Selector =
		| keyof HTMLElementTagNameMap
		| keyof SVGElementTagNameMap
		| string;
	export function waitForElement<K extends keyof HTMLElementTagNameMap>(
		selector: K,
		interval?: number,
	): Promise<HTMLElementTagNameMap[K] | null>;
	export function waitForElement<K extends keyof SVGElementTagNameMap>(
		selector: K,
		interval?: number,
	): Promise<SVGElementTagNameMap[K] | null>;
	export function waitForElement<E extends Element = Element>(
		selector: string,
		interval?: number,
	): Promise<E | null>;
	export function waitForElement(selector: Selector, interval = 100) {
		return waitForFunction(() => document.querySelector(selector), interval);
	}

	/**
	 * 重复调用某函数，直到其返回任意真值，并返回该真值。
	 * @param func 函数
	 * @param interval 重复调用时间间隔
	 * @returns func函数返回值
	 */
	export function waitForFunction<T>(
		func: () => T,
		interval = 100,
	): Promise<T> {
		return new Promise((rs) => {
			const handle = setInterval(() => {
				const result = func();
				if (result) {
					clearInterval(handle);
					rs(result);
				}
			}, interval);
		});
	}

	/**
	 * 创建一个将在一定时间后resolve的promise
	 * @param ms 延迟时间，以毫秒为单位。
	 * @returns 将在ms毫秒后resolve的一个Promise
	 */
	export function delay(ms: number) {
		return new Promise((rs) => setTimeout(rs, ms));
	}

	/**
	 * 创建DOM的hyperscript函数
	 * @deprecated 早期未使用React时自写的h函数，已弃用。
	 * @param tag
	 * @param settings
	 * @param children
	 * @returns
	 */
	// rome-ignore lint/suspicious/noExplicitAny: 属性随意
	export function dom(tag: string, settings: any, ...children: HTMLElement[]) {
		const tmp = document.createElement(tag);
		if (settings.class) {
			for (const cl of settings.class) {
				tmp.classList.add(cl);
			}
			settings.class = undefined;
		}

		if (settings.style) {
			for (const cl in settings.style) {
				tmp.style[cl] = settings.style[cl];
			}
			settings.style = undefined;
		}

		for (const v in settings) {
			if (settings[v]) tmp[v] = settings[v];
		}

		for (const child of children) {
			if (child) tmp.appendChild(child);
		}
		return tmp;
	}
}
