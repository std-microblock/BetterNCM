import { betterncmFetch } from "./base";

const e = encodeURIComponent;

/**
 * 和外界的文件系统进行交互的接口
 */
export namespace fs {
	/**
	 * 异步读取指定文件夹路径下的所有文件和文件夹
	 * @param folderPath 需要读取的文件夹路径
	 * @returns 所有文件和文件夹的相对路径或绝对路径
	 */
	export async function readDir(folderPath: string): Promise<string[]> {
		const r = await betterncmFetch(`/fs/read_dir?path=${e(folderPath)}`);
		return await r.json();
	}

	/**
	 * 读取文本文件，务必保证文件编码是 UTF-8
	 * @param filePath 需要读取的文件路径
	 * @returns 对应文件的文本形式
	 */
	export async function readFileText(filePath: string): Promise<string> {
		const r = await betterncmFetch(`/fs/read_file_text?path=${e(filePath)}`);
		return await r.text();
	}

	/**
	 * 读取文件
	 * @param filePath 需要读取的文件路径
	 * @returns blob
	 */
	export async function readFile(filePath: string): Promise<Blob> {
		const r = await betterncmFetch(`/fs/read_file?path=${e(filePath)}`);
		return await r.blob();
	}

	/**
	 * 挂载路径
	 * @param filePath 需要挂载的文件夹路径
	 * @returns 挂载到的 http 地址
	 */
	export async function mountDir(filePath: string): Promise<string> {
		const r = await betterncmFetch(`/fs/mount_dir?path=${e(filePath)}`);
		return await r.text();
	}

	/**
	 * 挂载路径
	 * @param filePath 需要挂载的文件路径
	 * @returns 挂载到的 http 地址
	 */
	 export async function mountFile(filePath: string): Promise<string> {
		const r = await betterncmFetch(`/fs/mount_file?path=${e(filePath)}`);
		return await r.text();
	}

	/**
	 * 解压指定的 ZIP 压缩文件到一个指定的文件夹中
	 * @param zipPath 需要解压的 ZIP 压缩文件路径
	 * @param unzipDest 需要解压到的文件夹路径，如果不存在则会创建，如果解压时有文件存在则会被覆盖
	 * @returns 返回值，是否成功
	 */
	export async function unzip(
		zipPath: string,
		unzipDest: string = `${zipPath}_extracted/`,
	): Promise<boolean> {
		const r = await betterncmFetch(
			`/fs/unzip_file?path=${e(zipPath)}&dest=${e(unzipDest)}`,
		);
		return parseInt(await r.text()) === 0;
	}

	/**
	 * 将文本写入到指定文件内
	 * @param filePath 需要写入的文件路径
	 * @param content 需要写入的文件内容
	 * @returns 是否成功
	 */
	export async function writeFileText(filePath: string, content: string) {
		const r = await betterncmFetch(`/fs/write_file_text?path=${e(filePath)}`, {
			method: "POST",
			body: content,
		});
		return r.status === 200;
	}

	/**
	 * 将文本或二进制数据写入到指定文件内
	 * @param filePath 需要写入的文件路径
	 * @param content 需要写入的文件内容
	 * @returns 是否成功
	 */
	export async function writeFile(filePath: string, content: string | Blob) {
		const fd = new FormData();
		fd.append("file", content);
		const r = await betterncmFetch(`/fs/write_file?path=${e(filePath)}`, {
			method: "POST",
			body: fd,
		});
		return r.status === 200;
	}

	/**
	 * 在指定路径新建文件夹
	 * @param dirPath 文件夹的路径
	 * @returns 是否成功
	 */
	export async function mkdir(dirPath: string) {
		const r = await betterncmFetch(`/fs/mkdir?path=${e(dirPath)}`);
		return r.status === 200;
	}

	/**
	 * 检查指定路径下是否存在文件或文件夹
	 * @param path 文件或文件夹的路径
	 * @returns 是否存在
	 */
	export async function exists(path: string): Promise<boolean> {
		const r = await betterncmFetch(`/fs/exists?path=${e(path)}`);
		return await r.json();
	}

	/**
	 * 删除指定路径下的文件或文件夹
	 * @param path 指定的文件或文件夹路径
	 */
	export async function remove(path: string) {
		const r = await betterncmFetch(`/fs/remove?path=${e(path)}`);
		return r.status === 200;
	}
}
