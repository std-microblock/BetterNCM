import { fs } from "./fs";

export namespace tests {
	export async function fail(reason: string) {
		console.warn("Test Failed", reason);
		await fs.writeFileText("/__TEST_FAILED__.txt", reason);
	}

	export async function success(message: string) {
		console.warn("Test Succeeded", message);
		await fs.writeFileText("/__TEST_SUCCEEDED__.txt", message);
	}
}
