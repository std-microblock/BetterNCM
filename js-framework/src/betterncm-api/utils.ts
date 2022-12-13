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

	export function delay(ms: number) {
		return new Promise((rs) => setTimeout(rs, ms));
	}

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
