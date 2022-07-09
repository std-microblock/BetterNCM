window['nbundle-build-debf8a4f-16b5-4703-af62-82be6251203d']={"nbundle-string-match-<16a7651f-51d4-465e-baa5-10303aba7ad7>":"<div class=\"live-songplayer invisible\">\r\n    <link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\r\n    <link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\r\n    <link href=\"https://fonts.googleapis.com/css2?family=Noto+Sans+SC:wght@100;300;400;500;700;900&display=swap\" rel=\"stylesheet\">\r\n\r\n    <div class=\"song-info\">\r\n        <div class=\"cover\">\r\n            <img class=\"cover-img\">\r\n        </div>\r\n        <div class=\"info\">\r\n            <div class=\"name\">TITLE</div>\r\n            <div class=\"author\">AUTHOR</div>\r\n        </div>\r\n        <div class=\"playtime\">\r\n            <span class=\"text\">00:00/00:00</span>\r\n            <span class=\"baar\">\r\n                <span class=\"innerbar\"></span>\r\n            </span>\r\n        </div>\r\n    </div>\r\n    <div class=\"wplaylist\">\r\n        \r\n    </div>\r\n</div>\r\n\r\n<div class=\"lsp-switcher on\" onclick=\"lsp_on()\">\r\n    Live SongPlayer（点击切换显示）\r\n</div>\r\n\r\n<div class=\"lsp-settings-btn\" onclick=\"lsp_settings()\">\r\n    设置\r\n</div>\r\n<style>\r\n    .lsp-switcher.off{\r\n        background: rgba(255, 0, 0, 0.253);\r\n    }\r\n\r\n    .lsp-switcher.on{\r\n        background: rgba(30, 255, 0, 0.281);\r\n    }\r\n\r\n    .lsp-switcher{\r\n        user-select: none;\r\n        cursor: pointer;\r\n        padding: 3px;\r\n        z-index: 999;\r\n        position: absolute;\r\n        right: 0;\r\n        bottom: 0;\r\n        margin:5px;\r\n        border-radius: 2px;\r\n    }\r\n\r\n    .lsp-settings-btn{\r\n        margin:5px;\r\n        user-select: none;\r\n        cursor: pointer;\r\n        padding: 3px;\r\n        z-index: 999;\r\n        position: absolute;\r\n        right: 200px;\r\n        bottom: 0;\r\n        background: rgb(0, 119, 255);\r\n        border-radius: 2px;\r\n    }\r\n\r\n    .invisible{\r\n        display: none !important;\r\n    }\r\n</style>\r\n\r\n<style>\r\n    .live-songplayer {\n  position: absolute;\n  right: 0;\n  top: 0;\n  width: 500px;\n  height: 140px;\n  background: #00ff00;\n  z-index: 999;\n  padding: 10px;\n}\n.live-songplayer .song-info {\n  height: calc(100% - 20px);\n  padding: 10px;\n  width: 350px;\n  display: inline-block;\n  float: left;\n  color: white;\n  border-radius: 6px;\n  background: #242424;\n}\n.live-songplayer .song-info .cover {\n  width: 80px;\n  height: 80px;\n  background: white;\n  border-radius: 7px;\n  float: left;\n  overflow: hidden;\n}\n.live-songplayer .song-info .cover img {\n  width: 100%;\n  height: 100%;\n}\n.live-songplayer .song-info .info {\n  height: 80px;\n  width: 240px;\n  overflow: hidden;\n  padding: 10px;\n  float: left;\n}\n.live-songplayer .song-info .info .name {\n  font-size: 18px;\n  font-weight: 800;\n  width: 240px;\n  overflow: hidden;\n}\n.live-songplayer .song-info .playtime .text {\n  font-size: 12px;\n  color: #969696;\n}\n.live-songplayer .song-info .playtime .baar {\n  position: relative;\n  width: 275px;\n  height: 4px;\n  border-radius: 10px;\n  display: inline-block;\n  background: #b4dbff;\n  overflow: hidden;\n  margin-bottom: 2px;\n}\n.live-songplayer .song-info .playtime .baar .innerbar {\n  position: absolute;\n  background: #006be6;\n  display: inline-block;\n  width: 100%;\n  height: 100%;\n}\n.live-songplayer .wplaylist {\n  float: right;\n  border-radius: 6px;\n  background: #242424;\n  height: calc(100% - 20px);\n  padding: 10px;\n  overflow: hidden;\n  margin: 0px 3px;\n  width: 100px;\n}\n.live-songplayer .wplaylist .line .name {\n  text-align: left;\n  overflow: hidden;\n  font-weight: 700;\n}\n.live-songplayer .wplaylist .line .user {\n  text-align: right;\n}\n\r\n</style>\r\n"}; // NBundle Datas

if (window["__lsp__loadflag__"])
    throw Error("Panic!");
window["__lsp__loadflag__"] = true;
setInterval(function () {
    if (!document.querySelector(".lsp-div")) {
        var player = document.createElement("div");
        player.classList.add("lsp-div");
        player.innerHTML = window['nbundle-build-debf8a4f-16b5-4703-af62-82be6251203d']['nbundle-string-match-<16a7651f-51d4-465e-baa5-10303aba7ad7>'];
        document.body.appendChild(player);
    }
}, 100);
(function(){setInterval(function () {
    function $(sel) {
    return document.querySelector(sel);
}
function $$(sel) {
    return document.querySelectorAll(sel);
}
function searchSong(keyword) {
    return new Promise(function (resolve) {
        var handle = setInterval(function () {
            if (window["searching"])
                return;
            window["searching"] = true;
            clearInterval(handle);
            $(".j-search-input").value = keyword;
            $(".sch-btn").click();
            var searchsong_handle = setInterval(function () {
                if (!$(".m-search .j-item"))
                    return;
                clearInterval(searchsong_handle);
                resolve(getSearchResult());
                setTimeout(function () {
                    window["searching"] = false;
                }, 50);
            }, 100);
        });
    });
}
function getSearchResult(number) {
    if (number === void 0) { number = 0; }
    return {
        name: $$(".m-search .j-item .title")[number].innerText.trim(),
        author: $$(".m-search .j-item .flow .s-fc3")[number].innerText.trim(),
        id: $$(".container-searchtrack .m-plylist .itm.f-cb.j-item.j-impress")[number].classList.toString().match(/tid-(\S+)/)[1].trim()
    };
}
function playAllSong() {
    $(".m-search.q-lrc.g-wrap1 .container.j-flag .oper.g-wrap5.j-flag .u-ibtn5b.u-ibtn5b-new.j-oper .u-ibtn5-ply").click();
}
function addAllToNext() {
    $(".u-ibtn5-new.u-ibtn5-only.u-ibtn5-addto").click();
}
function nextSong() {
    $(".btnc-nxt").click();
}
function pause() {
    $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause").click();
}
function is_paused() {
    return $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause") != null;
}
function play() {
    $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp.btnp-play.f-cp").click();
}
function formatTime(time) {
    return parseInt(time.split(":")[0]) * 60 + parseInt(time.split(":")[1]);
}
function getPlayTimePercent() {
    return formatTime($("time.now").innerText) / formatTime($("time.all").innerText);
}
function subscribeSongChanged(callback) {
    var lastcover = $(".u-cover.u-cover-sm img").src;
    setInterval(function () {
        var nowcover;
        if ($(".u-cover.u-cover-sm img") == null) {
            nowcover = "[[ NO COVER ]]";
        }
        else {
            nowcover = $(".u-cover.u-cover-sm img").src;
        }
        if (nowcover != lastcover) {
            lastcover = nowcover;
            callback();
        }
    }, 40);
}
function getPlaying() {
    var endfix = "";
    if ($(".bar .title * *"))
        endfix = " * *";
    if ($(".u-cover.u-cover-sm img") != null)
        return {
            cover: $(".u-cover.u-cover-sm img").src,
            name: ($(".bar .title" + endfix) || $(".bar .title")).innerText.split("\n")[0].trim(),
            author: ($(".bar P.j-title" + endfix) || $(".bar P.j-title")).innerText.split("\n")[0].trim()
        };
    else
        return {};
}
function findNativeFunction(obj, identifiers) {
    for (var key in obj) {
        var flag = true;
        for (var _i = 0, identifiers_1 = identifiers; _i < identifiers_1.length; _i++) {
            var identifier = identifiers_1[_i];
            if (!obj[key].toString().includes(identifier))
                flag = false;
        }
        if (flag)
            return key;
    }
}

    var img = $(".live-songplayer img.cover-img"), cimg = $(".u-cover.u-cover-sm img");
    img.src = cimg.src;
    var title = $(".bar .title"), author = $(".bar P.j-title");
    $(".live-songplayer .info .name").innerText = title.innerText.split("\n")[0];
    $(".live-songplayer .info .author").innerText = author.innerText.split("\n")[0];
    $(".live-songplayer .playtime .innerbar").style.width = (getPlayTimePercent() * 100).toFixed(6) + "%";
    $(".live-songplayer .playtime .text").innerText = $("time.now").innerText + "/" + $("time.all").innerText;
}, 100);
})()
/**
 * 待更换到WebSocket连接，省事先用HTTP了
 */
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
function onDanmaku(roomid, callback, ifcontinue) {
    if (roomid === void 0) { roomid = 790410; }
    if (callback === void 0) { callback = function (danmaku) { }; }
    if (ifcontinue === void 0) { ifcontinue = function () { return true; }; }
    return __awaiter(this, void 0, void 0, function () {
        var history, data, _i, _a, danmaku;
        var _this = this;
        return __generator(this, function (_b) {
            switch (_b.label) {
                case 0:
                    history = {};
                    return [4 /*yield*/, fetch("https://api.live.bilibili.com/xlive/web-room/v1/dM/gethistory?roomid=" + roomid)];
                case 1: return [4 /*yield*/, (_b.sent()).json()];
                case 2:
                    data = _b.sent();
                    for (_i = 0, _a = data.data.room; _i < _a.length; _i++) {
                        danmaku = _a[_i];
                        history[danmaku.text + danmaku.timeline + danmaku.nickname] = true;
                    }
                    setInterval(function () { return __awaiter(_this, void 0, void 0, function () {
                        var data, _i, _a, danmaku;
                        return __generator(this, function (_b) {
                            switch (_b.label) {
                                case 0:
                                    if (!ifcontinue())
                                        return [2 /*return*/];
                                    return [4 /*yield*/, fetch("https://api.live.bilibili.com/xlive/web-room/v1/dM/gethistory?roomid=" + roomid)];
                                case 1: return [4 /*yield*/, (_b.sent()).json()];
                                case 2:
                                    data = _b.sent();
                                    for (_i = 0, _a = data.data.room; _i < _a.length; _i++) {
                                        danmaku = _a[_i];
                                        if (!history[danmaku.text + danmaku.timeline + danmaku.nickname]) {
                                            history[danmaku.text + danmaku.timeline + danmaku.nickname] = true;
                                            callback(danmaku);
                                        }
                                    }
                                    return [2 /*return*/];
                            }
                        });
                    }); }, 3000);
                    return [2 /*return*/];
            }
        });
    });
}

var commands = [];
onDanmaku(localStorage["__lsp__live__id__"], function (danmaku) {
    var message = {
        command: null,
        arguments: [],
        sender: danmaku.nickname,
        origin: danmaku.text.trim()
    };
    for (var _i = 0, commands_1 = commands; _i < commands_1.length; _i++) {
        var command = commands_1[_i];
        var name = command.name;
        var callback = command.callback;
        var names = [];
        if (name instanceof Array)
            names = name;
        else
            names.push(name);
        for (var _a = 0, names_1 = names; _a < names_1.length; _a++) {
            var name_1 = names_1[_a];
            if (message.origin.startsWith(name_1)) {
                message.command = name_1;
                message.arguments = message.origin.slice(name_1.length).trim().split(" ");
                callback(message);
                return;
            }
        }
    }
}, function () { return document.querySelector(".live-songplayer.invisible") == null; });
(function () {
    function addCommand(name, callback) {
        commands.push({ name: name, callback: callback });
    }
    (function(){var __awaiter = (this && this.__awaiter) || function (thisArg, _arguments, P, generator) {
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
var _this = this;
function $(sel) {
    return document.querySelector(sel);
}
function $$(sel) {
    return document.querySelectorAll(sel);
}
function searchSong(keyword) {
    return new Promise(function (resolve) {
        var handle = setInterval(function () {
            if (window["searching"])
                return;
            window["searching"] = true;
            clearInterval(handle);
            $(".j-search-input").value = keyword;
            $(".sch-btn").click();
            var searchsong_handle = setInterval(function () {
                if (!$(".m-search .j-item"))
                    return;
                clearInterval(searchsong_handle);
                resolve(getSearchResult());
                setTimeout(function () {
                    window["searching"] = false;
                }, 50);
            }, 100);
        });
    });
}
function getSearchResult(number) {
    if (number === void 0) { number = 0; }
    return {
        name: $$(".m-search .j-item .title")[number].innerText.trim(),
        author: $$(".m-search .j-item .flow .s-fc3")[number].innerText.trim(),
        id: $$(".container-searchtrack .m-plylist .itm.f-cb.j-item.j-impress")[number].classList.toString().match(/tid-(\S+)/)[1].trim()
    };
}
function playAllSong() {
    $(".m-search.q-lrc.g-wrap1 .container.j-flag .oper.g-wrap5.j-flag .u-ibtn5b.u-ibtn5b-new.j-oper .u-ibtn5-ply").click();
}
function addAllToNext() {
    $(".u-ibtn5-new.u-ibtn5-only.u-ibtn5-addto").click();
}
function nextSong() {
    $(".btnc-nxt").click();
}
function pause() {
    $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause").click();
}
function is_paused() {
    return $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause") != null;
}
function play() {
    $(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp.btnp-play.f-cp").click();
}
function formatTime(time) {
    return parseInt(time.split(":")[0]) * 60 + parseInt(time.split(":")[1]);
}
function getPlayTimePercent() {
    return formatTime($("time.now").innerText) / formatTime($("time.all").innerText);
}
function subscribeSongChanged(callback) {
    var lastcover = $(".u-cover.u-cover-sm img").src;
    setInterval(function () {
        var nowcover;
        if ($(".u-cover.u-cover-sm img") == null) {
            nowcover = "[[ NO COVER ]]";
        }
        else {
            nowcover = $(".u-cover.u-cover-sm img").src;
        }
        if (nowcover != lastcover) {
            lastcover = nowcover;
            callback();
        }
    }, 40);
}
function getPlaying() {
    var endfix = "";
    if ($(".bar .title * *"))
        endfix = " * *";
    if ($(".u-cover.u-cover-sm img") != null)
        return {
            cover: $(".u-cover.u-cover-sm img").src,
            name: ($(".bar .title" + endfix) || $(".bar .title")).innerText.split("\n")[0].trim(),
            author: ($(".bar P.j-title" + endfix) || $(".bar P.j-title")).innerText.split("\n")[0].trim()
        };
    else
        return {};
}
function findNativeFunction(obj, identifiers) {
    for (var key in obj) {
        var flag = true;
        for (var _i = 0, identifiers_1 = identifiers; _i < identifiers_1.length; _i++) {
            var identifier = identifiers_1[_i];
            if (!obj[key].toString().includes(identifier))
                flag = false;
        }
        if (flag)
            return key;
    }
}

var playlist = [], playing = { name: "", author: "" };
function updatePlaylist() {
    var wplaylist = document.querySelector(".wplaylist");
    if (wplaylist) {
        wplaylist.innerHTML = playlist.map(function (v, i) { return "\n        <div class=\"line\">\n            <div class=\"name\">" + v.songinfo.name + "</div>\n            <div class=\"user\">" + v.sender + "</div>\n        </div>\n        "; }).join("");
    }
}
function switchsong() {
    return __awaiter(this, void 0, void 0, function () {
        var play, _playsong_;
        return __generator(this, function (_a) {
            // alert("switchsong+"+playlist.length)
            if (playlist.length > 0) {
                //    prompt("",`${JSON.stringify(playing)},${JSON.stringify(getPlaying())}`)
                if (playing.name.replace(/\s/g, "") != getPlaying().name.replace(/\s/g, "") || playing.author.replace(/\s/g, "") != getPlaying().author.replace(/\s/g, "")) {
                    play = playlist.shift();
                    _playsong_ = findNativeFunction(ctl.actionManager, [".logSource=", ".actionType!==undefined){"]);
                    ctl.actionManager[_playsong_]({ id: play.songinfo.id, type: "4", action: "play", from: 0, href: "", data: {} });
                    playing.name = play.songinfo.name.replace(/\s/g, "");
                    playing.author = play.songinfo.author.replace(/\s/g, "");
                    updatePlaylist();
                }
            }
            return [2 /*return*/];
        });
    });
}
subscribeSongChanged(switchsong);
addCommand(["play", "点歌", "pl"], function (message) { return __awaiter(_this, void 0, void 0, function () {
    var songinfo;
    return __generator(this, function (_a) {
        switch (_a.label) {
            case 0: return [4 /*yield*/, searchSong(message.arguments.join(" "))];
            case 1:
                songinfo = _a.sent();
                playlist.push({ songinfo: songinfo, sender: message.sender });
                updatePlaylist();
                switchsong();
                return [2 /*return*/];
        }
    });
}); });
})()
})();

function lsp_on(){
    document.querySelector(".live-songplayer").classList.remove("invisible");
    document.querySelector(".lsp-switcher").classList.remove("on");
    document.querySelector(".lsp-switcher").classList.add("off")
    document.querySelector(".lsp-switcher").onclick=lsp_off
}
function lsp_off(){
    document.querySelector(".live-songplayer").classList.add("invisible")
    document.querySelector(".lsp-switcher").classList.remove("off");
    document.querySelector(".lsp-switcher").classList.add("on")
    document.querySelector(".lsp-switcher").onclick=lsp_on
}

function lsp_settings(){
    localStorage["__lsp__live__id__"]=prompt("请输入您的直播间ID号",localStorage["__lsp__live__id__"])||localStorage["__lsp__live__id__"]
    document.location.reload()
}
