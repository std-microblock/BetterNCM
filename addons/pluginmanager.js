"use strict";
var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
    function adopt(value) { return value instanceof P ? value : new P(function (resolve) { resolve(value); }); }
    return new (P || (P = Promise))(function (resolve, reject) {
        function fulfilled(value) { try { step(generator.next(value)); } catch (e) { reject(e); } }
        function rejected(value) { try { step(generator["throw"](value)); } catch (e) { reject(e); } }
        function step(result) { result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected); }
        step((generator = generator.apply(thisArg, _arguments || [])).next());
    });
};
var __generator = (this && this.__generator) || function (thisArg, body) {
    var _ = { label: 0, sent: function() { if (t[0] & 1) throw t[1]; return t[1]; }, trys: [], ops: [] }, f, y, t, g;
    return g = { next: verb(0), "throw": verb(1), "return": verb(2) }, typeof Symbol === "function" && (g[Symbol.iterator] = function() { return this; }), g;
    function verb(n) { return function (v) { return step([n, v]); }; }
    function step(op) {
        if (f) throw new TypeError("Generator is already executing.");
        while (_) try {
            if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
            if (y = 0, t) op = [op[0] & 2, t.value];
            switch (op[0]) {
                case 0: case 1: t = op; break;
                case 4: _.label++; return { value: op[1], done: false };
                case 5: _.label++; y = op[1]; op = [0]; continue;
                case 7: op = _.ops.pop(); _.trys.pop(); continue;
                default:
                    if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) { _ = 0; continue; }
                    if (op[0] === 3 && (!t || (op[1] > t[0] && op[1] < t[3]))) { _.label = op[1]; break; }
                    if (op[0] === 6 && _.label < t[1]) { _.label = t[1]; t = op; break; }
                    if (t && _.label < t[2]) { _.label = t[2]; _.ops.push(op); break; }
                    if (t[2]) _.ops.pop();
                    _.trys.pop(); continue;
            }
            op = body.call(thisArg, _);
        } catch (e) { op = [6, e]; y = 0; } finally { f = t = 0; }
        if (op[0] & 5) throw op[1]; return { value: op[0] ? op[1] : void 0, done: true };
    }
};
var onlinePlugins = [];
var postInstall = function (ind) {
    return __awaiter(this, void 0, void 0, function () {
        var plugin, content, e_1;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    plugin = onlinePlugins[ind];
                    return [4 /*yield*/, fetch(plugin["url"])];
                case 1: return [4 /*yield*/, (_a.sent()).text()];
                case 2:
                    content = _a.sent();
                    _a.label = 3;
                case 3:
                    _a.trys.push([3, 6, , 7]);
                    return [4 /*yield*/, fetch("http://music.163.com/betterncm_api/write_file?" + plugin["filepath"], { body: content, method: "POST" })];
                case 4: return [4 /*yield*/, (_a.sent()).text()];
                case 5:
                    _a.sent();
                    return [3 /*break*/, 7];
                case 6:
                    e_1 = _a.sent();
                    alert(e_1);
                    return [3 /*break*/, 7];
                case 7:
                    document.location.reload();
                    return [2 /*return*/];
            }
        });
    });
};
var postUninstall = function (ind) {
    return __awaiter(this, void 0, void 0, function () {
        var plugin;
        return __generator(this, function (_a) {
            switch (_a.label) {
                case 0:
                    plugin = onlinePlugins[ind];
                    return [4 /*yield*/, fetch("http://music.163.com/betterncm_api/write_file?" + plugin["filepath"], { body: "", method: "POST" })];
                case 1: return [4 /*yield*/, (_a.sent()).text()];
                case 2:
                    _a.sent();
                    document.location.reload();
                    return [2 /*return*/];
            }
        });
    });
};
(function () { return __awaiter(void 0, void 0, void 0, function () {
    var handle, e_2, _i, onlinePlugins_1, plugin, script, e_3;
    return __generator(this, function (_a) {
        switch (_a.label) {
            case 0:
                _a.trys.push([0, 3, , 4]);
                return [4 /*yield*/, fetch("https://ganbei-hot-update-1258625969.file.myqcloud.com/betterncm/online_addons.json?" +
                        new Date().getTime())];
            case 1: return [4 /*yield*/, (_a.sent()).json()];
            case 2:
                onlinePlugins = _a.sent();
                return [3 /*break*/, 4];
            case 3:
                e_2 = _a.sent();
                return [3 /*break*/, 4];
            case 4:
                _i = 0, onlinePlugins_1 = onlinePlugins;
                _a.label = 5;
            case 5:
                if (!(_i < onlinePlugins_1.length)) return [3 /*break*/, 12];
                plugin = onlinePlugins_1[_i];
                script = void 0;
                _a.label = 6;
            case 6:
                _a.trys.push([6, 9, , 10]);
                return [4 /*yield*/, fetch("http://music.163.com/betterncm_api/read_file?" + plugin["filepath"])];
            case 7: return [4 /*yield*/, (_a.sent()).text()];
            case 8:
                script = _a.sent();
                return [3 /*break*/, 10];
            case 9:
                e_3 = _a.sent();
                script = "";
                return [3 /*break*/, 10];
            case 10:
                // alert(script)
                if (script != "")
                    plugin["installed"] = true;
                _a.label = 11;
            case 11:
                _i++;
                return [3 /*break*/, 5];
            case 12:
                handle = setInterval(function () {
                    var ele = document.querySelector(".g-mn-set");
                    if (ele && !document.querySelector("#betterncmPM")) {
                        var settings = document.createElement("div");
                        settings.style.padding = "20px";
                        settings.style.width = "100%";
                        settings.style.zIndex = "100000";
                        settings.id = "betterncmPM";
                        settings.innerHTML = "\n      <h3 class='s-fc1 f-ff2' style='font-size:24px;font-weight:800;margin:20px;'>BetterNCM</h3>\n      \n      <button class='u-ibtn5' onclick='fetch(\"https://music.163.com/betterncm_api/openconfigfolder\")'>\n      \u6253\u5F00\u914D\u7F6E\u6587\u4EF6\u5939</button>\n      \n      <button class='u-ibtn5' onclick='document.location.reload()'>\u91CD\u8F7D\u9875\u9762</button>\n      \n      <h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM \u63D2\u4EF6\u5217\u8868</h3>\n      <div class=\"onlinepluginlish\">\n          " + onlinePlugins
                            .map(function (plugin, ind) {
                            return "<div>\n                <span style=\"font-size:16px;font-weight:600;\">" + plugin["name"] + "</span>\n                <span style=\"font-size:14px;font-weight:500;color:gray;\">(" + plugin["author"] + ")</span>\n                <button class='u-ibtn5' style=\"display:" + (plugin["installed"] ? "none" : "inline-block") + ";\" onclick=\"postInstall(" + ind + ");\">\u5B89\u88C5</button>\n                <button class='u-ibtn5' style=\"display:" + (plugin["installed"] ? "inline-block" : "none") + ";\" onclick=\"postUninstall(" + ind + ");\">\u5378\u8F7D</button>\n                <div style=\"padding:5px;\">\n                " + plugin["description"] + "\n                </div>\n            </div><br>";
                        })
                            .join("") + "\n      </div>";
                        ele.appendChild(settings);
                    }
                }, 100);
                return [2 /*return*/];
        }
    });
}); })();
