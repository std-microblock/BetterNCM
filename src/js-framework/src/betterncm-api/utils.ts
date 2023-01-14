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
	 * 将指定的函数做防抖处理
	 * @param callback 需要被调用的回调函数
	 * @param waitTime 需要等待多长时间，单位毫秒
	 * @returns 包装后的防抖函数
	 */
	export function debounce<T extends Function>(
		callback: T,
		waitTime: number,
	): T {
		let timer = 0;
		return function debounceClosure() {
			const self = this;
			// rome-ignore lint/style/noArguments: 防抖函数
			const args = arguments;
			if (timer) {
				clearTimeout(timer);
			}
			timer = setTimeout(callback.bind(self, args), waitTime);
		} as unknown as T;
	}

	/**
	 * 重复调用某函数，直到其返回任意真值，并返回该真值。
	 * @param func 函数
	 * @param interval 重复调用时间间隔
	 * @returns `func` 函数的返回值
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
	 * 创建一个将在一定时间后 resolve 的 Promise
	 * @param ms 延迟时间，以毫秒为单位。
	 * @returns 将在ms毫秒后resolve的一个Promise
	 */
	export function delay(ms: number) {
		return new Promise((rs) => setTimeout(rs, ms));
	}

	/**
	 * 简易的创建一个元素的函数
	 * @deprecated 早期未使用 React 时写的辅助函数，已弃用，请考虑使用自带的 React 构建复杂页面。
	 * @param tag 元素类型
	 * @param settings 元素的属性键值对
	 * @param children 元素的子元素，按顺序添加
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
