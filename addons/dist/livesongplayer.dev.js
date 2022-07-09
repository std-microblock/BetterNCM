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

var lsp_lastsongname = "_";

(function () {
  return __awaiter(void 0, void 0, void 0, function () {
    function playSong(id) {
      document.querySelector(".j-search-input").value = id;
      document.querySelector(".sch-btn").click();
      var lsp_pl_handle = setInterval(function () {
        if (!document.querySelector(".m-search .j-item")) return;
        playAllSong();
        clearInterval(lsp_pl_handle);
        setTimeout(function () {
          if (document.querySelector(".j-title")) lsp_lastsongname = document.querySelector(".j-title").innerText;
          lsp_polling = false;
          play(); //   if(document.querySelector(".fmdelete"))playAllSong()
        }, 300);
      }, 100);
    }

    function poll_next_song() {
      if (lsp_polling) return;
      lsp_polling = true;
      fetch("http://music.163.com:10103/poll_song").then(function (v) {
        return v.json();
      }).then(function (t) {
        playSong(t.id);
      });
    }

    function playAllSong() {
      document.querySelector(".m-search.q-lrc.g-wrap1 .container.j-flag .oper.g-wrap5.j-flag .u-ibtn5b.u-ibtn5b-new.j-oper .u-ibtn5-ply").click();
    }

    function pause() {
      document.querySelector(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause").click();
    }

    function play() {
      document.querySelector(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp.btnp-play.f-cp").click();
    }

    function formatTime(time) {
      return parseInt(time.split(":")[0] * 60) + parseInt(time.split(":")[1]);
    }

    function getPlayTimePercent() {
      return formatTime(document.querySelector("time.now").innerText) / formatTime(document.querySelector("time.all").innerText);
    }

    var t, lsp_polling;
    return __generator(this, function (_a) {
      switch (_a.label) {
        case 0:
          return [4
          /*yield*/
          , fetch("http://music.163.com:10103/")];

        case 1:
          return [4
          /*yield*/
          , _a.sent().text()];

        case 2:
          t = _a.sent();
          alert("网易云*点歌姬开始运行");
          lsp_polling = false;
          setInterval(function () {
            fetch("http://music.163.com:10103/sync?__time" + getPlayTimePercent() + "__").then(function (v) {
              return v.text();
            }).then(function (t) {
              if (t == "skip") poll_next_song();
            });
          }, 100);
          setInterval(function () {
            if (lsp_polling) return;

            if (getPlayTimePercent() >= 0.97 || lsp_lastsongname != document.querySelector(".j-title").innerText) {
              lsp_lastsongname = "";
              poll_next_song();
              pause();
            }
          }, 1000);
          return [2
          /*return*/
          ];
      }
    });
  });
})();