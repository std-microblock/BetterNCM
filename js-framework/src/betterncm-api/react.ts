import BetterNCM from ".";

function initNCMReact() {
	if ("React" in window) {
		if ("createElement" in React && "Fragment" in React) {
			window.h = React.createElement;
			window.f = React.Fragment;
			return true;
		}
	}
	return "h" in window && "f" in window;
}

BetterNCM.utils.waitForFunction(initNCMReact, 100);