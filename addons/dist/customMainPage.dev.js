"use strict";

var BetterNCMApi;

(function (BetterNCMApi) {
  var fs;

  (function (fs) {
    function readFile(path) {
      return regeneratorRuntime.async(function readFile$(_context) {
        while (1) {
          switch (_context.prev = _context.next) {
            case 0:
              _context.t0 = regeneratorRuntime;
              _context.next = 3;
              return regeneratorRuntime.awrap(fetch("http://music.163.com/betterncm_api/read_file?" + path));

            case 3:
              _context.t1 = _context.sent.text();
              _context.next = 6;
              return _context.t0.awrap.call(_context.t0, _context.t1);

            case 6:
              return _context.abrupt("return", _context.sent);

            case 7:
            case "end":
              return _context.stop();
          }
        }
      });
    }

    fs.readFile = readFile;

    function writeFile(path, content) {
      return regeneratorRuntime.async(function writeFile$(_context2) {
        while (1) {
          switch (_context2.prev = _context2.next) {
            case 0:
              _context2.t0 = regeneratorRuntime;
              _context2.next = 3;
              return regeneratorRuntime.awrap(fetch("http://music.163.com/betterncm_api/write_file?" + path, {
                body: content,
                method: "POST"
              }));

            case 3:
              _context2.t1 = _context2.sent.text();
              _context2.next = 6;
              return _context2.t0.awrap.call(_context2.t0, _context2.t1);

            case 6:
            case "end":
              return _context2.stop();
          }
        }
      });
    }

    fs.writeFile = writeFile;
  })(fs = BetterNCMApi.fs || (BetterNCMApi.fs = {}));
})(BetterNCMApi || (BetterNCMApi = {}));

var CustomMainPagePlugin;

(function (CustomMainPagePlugin) {
  function loadHTML(html) {
    document.querySelector(".g-mn.one div").innerHTML = html;
  }

  function parseContent(pageContent) {
    var dom = document.createElement(pageContent.base || "div");

    dom.onclick = function () {
      var _a;

      parseActionBlocks((_a = pageContent.action) === null || _a === void 0 ? void 0 : _a.click);
    };

    switch (pageContent.type) {
      case "text":
        {
          dom.innerText = pageContent.text;
          dom.style.fontSize = pageContent.size;
          dom.style.padding = pageContent.padding;
          dom.style.all = pageContent.style;
          break;
        }

      case "br":
        {
          dom = document.createElement("br");
          break;
        }

      case "playlist":
        {
          dom.innerHTML = "<li width=\"20\" style=\"width:120px\"><div  class=\"cvr u-cover u-cover-daily u-cover-recmd\"  data-res-menu=\"true\"  data-res-type=\"21\">\n                <a  href=\"#/m/dailysong/\"  class=\"lnk\"  data-da-event=\"c361\">\n                <img  src=\"orpheus://orpheus/style/res/common/discovery/calendar_bg.png\"/>\n                <div  class=\"date\">\n                <svg>\n                <use  xlink:href=\"orpheus://orpheus/style/res/svg/cover.sp.svg#calendar_box\"></use>\n                </svg>\n                <div  class=\"day\">\n                16\n                </div>\n                </div>\n                </a>\n                <div  class=\"reason f-pa f-tl\">\n                <p  class=\"f-thide2\">\n                \u6839\u636E\u60A8\u7684\u97F3\u4E50\u53E3\u5473\u751F\u6210\u6BCF\u65E5\u66F4\u65B0\n                </p>\n                </div>\n                <span  class=\"ply f-pa f-cp\"  data-res-action=\"play\"  data-res-type=\"21\"  data-log-action=\"playall\"  data-log-source=\"dailyrecommend\"  data-action=\"play\">\n                <svg>\n                <use  xlink:href=\"orpheus://orpheus/style/res/svg/cover.sp.svg#play_rb_30\"></use>\n                </svg>\n                </span>\n                </div></li>";
        }
    }

    return dom;
  }

  function parseBlock(block) {
    var _a;

    switch (block.type) {
      case "renamePage":
        {
          document.querySelector("nav .firstlist a").innerHTML = block.name;
          break;
        }

      case "setPageContent":
        {
          var a;
          var generated = document.createElement("div");
          generated.classList.add("cc-microblock-betterncm-pagecontent");
          var _iteratorNormalCompletion = true;
          var _didIteratorError = false;
          var _iteratorError = undefined;

          try {
            for (var _iterator = block.content[Symbol.iterator](), _step; !(_iteratorNormalCompletion = (_step = _iterator.next()).done); _iteratorNormalCompletion = true) {
              var content = _step.value;
              generated.appendChild(parseContent(content));
            }
          } catch (err) {
            _didIteratorError = true;
            _iteratorError = err;
          } finally {
            try {
              if (!_iteratorNormalCompletion && _iterator["return"] != null) {
                _iterator["return"]();
              }
            } finally {
              if (_didIteratorError) {
                throw _iteratorError;
              }
            }
          }

          (_a = document.querySelector(".g-mn.one").children[0]) === null || _a === void 0 ? void 0 : _a.remove();
          document.querySelector(".g-mn.one").appendChild(generated);
          break;
        }

      case "setHash":
        {
          document.location.hash = block.hash;
          break;
        }

      case "alert":
        {
          alert(block.text);
          break;
        }
    }
  }

  function parseActionBlocks(data) {
    var _iteratorNormalCompletion2 = true;
    var _didIteratorError2 = false;
    var _iteratorError2 = undefined;

    try {
      for (var _iterator2 = data[Symbol.iterator](), _step2; !(_iteratorNormalCompletion2 = (_step2 = _iterator2.next()).done); _iteratorNormalCompletion2 = true) {
        var block = _step2.value;
        parseBlock(block);
      }
    } catch (err) {
      _didIteratorError2 = true;
      _iteratorError2 = err;
    } finally {
      try {
        if (!_iteratorNormalCompletion2 && _iterator2["return"] != null) {
          _iterator2["return"]();
        }
      } finally {
        if (_didIteratorError2) {
          throw _iteratorError2;
        }
      }
    }
  }

  var handle = -1;

  function load() {
    var data;
    return regeneratorRuntime.async(function load$(_context3) {
      while (1) {
        switch (_context3.prev = _context3.next) {
          case 0:
            _context3.t0 = JSON;
            _context3.next = 3;
            return regeneratorRuntime.awrap(BetterNCMApi.fs.readFile("./customMainPage.json"));

          case 3:
            _context3.t1 = _context3.sent;
            data = _context3.t0.parse.call(_context3.t0, _context3.t1);
            clearInterval(handle);
            handle = setInterval(function () {
              if (document.querySelector(".g-mn.one .g-recmdwrap")) {
                parseActionBlocks(data);
              } else if (document.querySelectorAll(".g-mn.one>div").length > 1) {
                var pageContentDom = document.querySelector(".cc-microblock-betterncm-pagecontent");
                if (pageContentDom) pageContentDom.remove();
              }
            }, 100);

          case 7:
          case "end":
            return _context3.stop();
        }
      }
    });
  }

  CustomMainPagePlugin.load = load;
})(CustomMainPagePlugin || (CustomMainPagePlugin = {}));

CustomMainPagePlugin.load();