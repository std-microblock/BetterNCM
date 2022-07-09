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
window["configs"] = JSON.parse(localStorage["betterncm.cssloader.config"] || "{}");
window["saveCSSSettings"] = function () {
    localStorage["betterncm.cssloader.config"] = JSON.stringify(window["configs"]);
    document.location.reload();
};
var CSSLoader = /** @class */ (function () {
    function CSSLoader() {
    }
    CSSLoader.loadStyles = function () {
        return __awaiter(this, void 0, void 0, function () {
            var stylesheetsText, stylesheets, configsHTML, _loop_1, this_1, _i, stylesheets_1, stylesheetFile;
            return __generator(this, function (_a) {
                switch (_a.label) {
                    case 0: return [4 /*yield*/, fetch("https://music.163.com/betterncm_api/getdir?stylesheets/")];
                    case 1: return [4 /*yield*/, (_a.sent()).text()];
                    case 2:
                        stylesheetsText = _a.sent();
                        stylesheets = stylesheetsText.split(",");
                        configsHTML = "<h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM StyleSheet\u8BBE\u7F6E</h3>";
                        _loop_1 = function (stylesheetFile) {
                            var url, stylesheet, styleObj, style;
                            return __generator(this, function (_a) {
                                switch (_a.label) {
                                    case 0:
                                        if (!stylesheetFile.endsWith(".css"))
                                            return [2 /*return*/, "continue"];
                                        url = "https://music.163.com/betterncm_api/read_file?stylesheets/" +
                                            stylesheetFile;
                                        return [4 /*yield*/, fetch(url)];
                                    case 1: return [4 /*yield*/, (_a.sent()).text()];
                                    case 2:
                                        stylesheet = _a.sent();
                                        styleObj = this_1.parseStyle(stylesheet);
                                        configs[styleObj.get_name()] = configs[styleObj.get_name()] || styleObj.get_configs();
                                        configsHTML += "\n        <div>\n            <h3 style='font-size:16px;font-weight:700;'>" + styleObj.get_name() + "</h3>\n            " + Object.keys(styleObj.get_configs()).map(function (v) {
                                            return "<div>" + v + ":\n              <input class=\"txt u-txt __cssLoader__config__input__\" \n              onkeyup='window[\"configs\"][\"" + styleObj.get_name() + "\"][\"" + v + "\"]=event.target.value'\n              value=\"" + configs[styleObj.get_name()][v] + "\"></div>";
                                        }) + "\n            \n        </div>\n      ";
                                        setInterval(function () {
                                            var pm = document.querySelector("#betterncmPM");
                                            var cssConfig = document.querySelector("#betterncmCSSConf");
                                            if (pm && !cssConfig) {
                                                var conf = document.createElement("div");
                                                conf.id = "betterncmCSSConf";
                                                conf.innerHTML = configsHTML;
                                                pm.appendChild(conf);
                                            }
                                        }, 100);
                                        style = document.createElement("style");
                                        style.innerHTML = styleObj.generator(configs[styleObj.get_name()]);
                                        document.head.appendChild(style);
                                        return [2 /*return*/];
                                }
                            });
                        };
                        this_1 = this;
                        _i = 0, stylesheets_1 = stylesheets;
                        _a.label = 3;
                    case 3:
                        if (!(_i < stylesheets_1.length)) return [3 /*break*/, 6];
                        stylesheetFile = stylesheets_1[_i];
                        return [5 /*yield**/, _loop_1(stylesheetFile)];
                    case 4:
                        _a.sent();
                        _a.label = 5;
                    case 5:
                        _i++;
                        return [3 /*break*/, 3];
                    case 6:
                        configsHTML += "<button class='u-ibtn5' onclick=\"saveCSSSettings()\">\u786E\u5B9A</button>";
                        return [2 /*return*/];
                }
            });
        });
    };
    CSSLoader.parseStyle = function (stylesheetText) {
        var stylejson = /==\|\|((.|\n|\r)*)\|\|==/.exec(stylesheetText);
        if (!((stylesheetText.includes("[[ BetterNCM-CSS-V2 ]]") ||
            stylesheetText.includes("[[BetterNCM-CSS-V2]]")) &&
            stylejson))
            return {
                generator: function (config) {
                    return stylesheetText;
                },
                get_configs: function () {
                    return [];
                },
                get_name: function () {
                    return "Unnamed Stylesheet";
                }
            };
        var ssjson = JSON.parse(stylejson[1]);
        return {
            generator: function (cconfig) {
                var _text = stylesheetText;
                var configs = this.get_configs();
                for (var _i = 0, _a = Object.keys(configs); _i < _a.length; _i++) {
                    var config = _a[_i];
                    var configValue = cconfig[config];
                    if (configValue)
                        while (_text.includes("[[" + config + "]]"))
                            _text = _text.replace("[[" + config + "]]", configValue);
                }
                return _text;
            },
            get_configs: function () {
                return ssjson["configs"];
            },
            get_name: function () {
                return ssjson["name"];
            }
        };
    };
    return CSSLoader;
}());
CSSLoader.loadStyles();
