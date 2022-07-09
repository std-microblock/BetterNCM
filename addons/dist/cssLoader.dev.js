"use strict";

var __awaiter = void 0 && (void 0).__awaiter || function (thisArg, _arguments, P, generator) {
  function adopt(value) {
    return value instanceof P ? value : new P(function (resolve) {
      resolve(value);
    });
  }

  return new (P || (P = Promise))(function (resolve, reject) {
    function fulfilled(value) {
      try {
        step(generator.next(value));
      } catch (e) {
        reject(e);
      }
    }

    function rejected(value) {
      try {
        step(generator["throw"](value));
      } catch (e) {
        reject(e);
      }
    }

    function step(result) {
      result.done ? resolve(result.value) : adopt(result.value).then(fulfilled, rejected);
    }

    step((generator = generator.apply(thisArg, _arguments || [])).next());
  });
};

var __generator = void 0 && (void 0).__generator || function (thisArg, body) {
  var _ = {
    label: 0,
    sent: function sent() {
      if (t[0] & 1) throw t[1];
      return t[1];
    },
    trys: [],
    ops: []
  },
      f,
      y,
      t,
      g;
  return g = {
    next: verb(0),
    "throw": verb(1),
    "return": verb(2)
  }, typeof Symbol === "function" && (g[Symbol.iterator] = function () {
    return this;
  }), g;

  function verb(n) {
    return function (v) {
      return step([n, v]);
    };
  }

  function step(op) {
    if (f) throw new TypeError("Generator is already executing.");

    while (_) {
      try {
        if (f = 1, y && (t = op[0] & 2 ? y["return"] : op[0] ? y["throw"] || ((t = y["return"]) && t.call(y), 0) : y.next) && !(t = t.call(y, op[1])).done) return t;
        if (y = 0, t) op = [op[0] & 2, t.value];

        switch (op[0]) {
          case 0:
          case 1:
            t = op;
            break;

          case 4:
            _.label++;
            return {
              value: op[1],
              done: false
            };

          case 5:
            _.label++;
            y = op[1];
            op = [0];
            continue;

          case 7:
            op = _.ops.pop();

            _.trys.pop();

            continue;

          default:
            if (!(t = _.trys, t = t.length > 0 && t[t.length - 1]) && (op[0] === 6 || op[0] === 2)) {
              _ = 0;
              continue;
            }

            if (op[0] === 3 && (!t || op[1] > t[0] && op[1] < t[3])) {
              _.label = op[1];
              break;
            }

            if (op[0] === 6 && _.label < t[1]) {
              _.label = t[1];
              t = op;
              break;
            }

            if (t && _.label < t[2]) {
              _.label = t[2];

              _.ops.push(op);

              break;
            }

            if (t[2]) _.ops.pop();

            _.trys.pop();

            continue;
        }

        op = body.call(thisArg, _);
      } catch (e) {
        op = [6, e];
        y = 0;
      } finally {
        f = t = 0;
      }
    }

    if (op[0] & 5) throw op[1];
    return {
      value: op[0] ? op[1] : void 0,
      done: true
    };
  }
};

var CSSLoader =
/** @class */
function () {
  function CSSLoader() {}

  CSSLoader.loadStyles = function () {
    return __awaiter(this, void 0, void 0, function () {
      var stylesheetsText, stylesheets, configsHTML, _i, stylesheets_1, stylesheetFile, url, stylesheet, styleObj, config, style;

      return __generator(this, function (_a) {
        switch (_a.label) {
          case 0:
            return [4
            /*yield*/
            , fetch("https://music.163.com/betterncm_api/getdir?stylesheets/")];

          case 1:
            return [4
            /*yield*/
            , _a.sent().text()];

          case 2:
            stylesheetsText = _a.sent();
            stylesheets = stylesheetsText.split(",");
            configsHTML = "<h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM StyleSheet\u8BBE\u7F6E</h3>";
            _i = 0, stylesheets_1 = stylesheets;
            _a.label = 3;

          case 3:
            if (!(_i < stylesheets_1.length)) return [3
            /*break*/
            , 7];
            stylesheetFile = stylesheets_1[_i];
            if (!stylesheetFile.endsWith(".css")) return [3
            /*break*/
            , 6];
            url = "https://music.163.com/betterncm_api/read_file?stylesheets/" + stylesheetFile;
            return [4
            /*yield*/
            , fetch(url)];

          case 4:
            return [4
            /*yield*/
            , _a.sent().text()];

          case 5:
            stylesheet = _a.sent();
            styleObj = this.parseStyle(stylesheet);
            config = JSON.parse(localStorage["__cssLoader__" + stylesheetFile + "__config"] || "{}");
            configsHTML += "\n        <div>\n            <h3 style='font-size:16px;font-weight:700;'>" + styleObj.get_name() + "</h3>\n            " + styleObj.get_configs().map(function (v) {
              return "<div>" + v + ":<input class=\"txt u-txt __cssLoader__config__input__\"></div>";
            }) + "\n        </div>\n      ";
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
            style.innerHTML = styleObj.generator(config);
            document.head.appendChild(style);
            _a.label = 6;

          case 6:
            _i++;
            return [3
            /*break*/
            , 3];

          case 7:
            return [2
            /*return*/
            ];
        }
      });
    });
  };

  CSSLoader.parseStyle = function (stylesheetText) {
    var stylejson = /==\|\|((.|\n|\r)*)\|\|==/.exec(stylesheetText);
    if (!((stylesheetText.includes("[[ BetterNCM-CSS-V2 ]]") || stylesheetText.includes("[[BetterNCM-CSS-V2]]")) && stylejson)) return {
      generator: function generator(config) {
        return stylesheetText;
      },
      get_configs: function get_configs() {
        return [];
      },
      get_name: function get_name() {
        return "Unnamed Stylesheet";
      }
    };
    var ssjson = JSON.parse(stylejson[1]);
    return {
      generator: function generator(cconfig) {
        var _text = stylesheetText;
        var configs = this.get_configs();

        for (var _i = 0, configs_1 = configs; _i < configs_1.length; _i++) {
          var config = configs_1[_i];
          var configValue = cconfig.get(config);
          if (configValue) while (_text.includes("[[" + config + "]]")) {
            _text.replace("[[" + config + "]]", configValue);
          }
        }

        return stylesheetText;
      },
      get_configs: function get_configs() {
        return ssjson["configs"];
      },
      get_name: function get_name() {
        return ssjson["name"];
      }
    };
  };

  return CSSLoader;
}();

CSSLoader.loadStyles();