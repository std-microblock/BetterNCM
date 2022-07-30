"use strict";
var BetterNCMApi;
(function (BetterNCMApi) {
    let fs;
    (function (fs) {
        async function readFile(path) {
            return await (await fetch("http://music.163.com/betterncm_api/read_file?" + path)).text();
        }
        fs.readFile = readFile;
        async function writeFile(path, content) {
            await (await fetch("http://music.163.com/betterncm_api/write_file?" + path, { body: content, method: "POST" })).text();
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
        let dom = document.createElement(pageContent.base || "div");
        dom.onclick = () => {
            var _a;
            parseActionBlocks((_a = pageContent.action) === null || _a === void 0 ? void 0 : _a.click);
        };
        switch (pageContent.type) {
            case "text": {
                dom.innerText = pageContent.text;
                dom.style.fontSize = pageContent.size;
                dom.style.padding = pageContent.padding;
                dom.style.all = pageContent.style;
                break;
            }
            case "br": {
                dom = document.createElement("br");
                break;
            }
            case "playlist": {
                dom.innerHTML = `<div style="width:150px" class="cvr u-cover u-cover-daily u-cover-recmd"  data-res-menu="true"  data-res-type="21">
                <img  src="${pageContent.image}"/>
                <div  class="date">
                <svg>
                <use  xlink:href="orpheus://orpheus/style/res/svg/cover.sp.svg#calendar_box"></use>
                </svg>
                <div  class="day">
                16
                </div>
                </div>
                </a>
                <div  class="reason f-pa f-tl">
                <p  class="f-thide2">
                ${pageContent.text}
                </p>
                </div>
                <span  class="ply f-pa f-cp"  data-res-action="play"  data-res-type="21"  data-log-action="playall"  data-log-source="dailyrecommend"  data-action="play">
                <svg>
                <use  xlink:href="orpheus://orpheus/style/res/svg/cover.sp.svg#play_rb_30"></use>
                </svg>
                </span>
                </div>`;
            }
        }
        return dom;
    }
    async function parseBlock(block) {
        var _a;
        switch (block.type) {
            case "renamePage": {
                document.querySelector("nav .firstlist a").innerHTML = block.name;
                break;
            }
            case "setPageContent": {
                let a;
                let generated = document.createElement("div");
                generated.classList.add("cc-microblock-betterncm-pagecontent");
                for (let content of block.content) {
                    generated.appendChild(parseContent(content));
                }
                (_a = document.querySelector(".g-mn.one").children[0]) === null || _a === void 0 ? void 0 : _a.remove();
                document.querySelector(".g-mn.one").appendChild(generated);
                break;
            }
            case "setHash": {
                document.location.hash = block.hash;
                break;
            }
            case "alert": {
                alert(block.text);
                break;
            }
        }
    }
    function parseActionBlocks(data) {
        for (let block of data) {
            parseBlock(block);
        }
    }
    let handle = -1;
    async function load() {
        let data = JSON.parse(await BetterNCMApi.fs.readFile("./customMainPage.json"));
        clearInterval(handle);
        handle = setInterval(() => {
            if (document.querySelector(".g-mn.one .g-recmdwrap")) {
                parseActionBlocks(data);
            }
            else if (document.querySelectorAll(".g-mn.one>div").length > 1) {
                let pageContentDom = document.querySelector(".cc-microblock-betterncm-pagecontent");
                if (pageContentDom)
                    pageContentDom.remove();
            }
        }, 100);
    }
    CustomMainPagePlugin.load = load;
})(CustomMainPagePlugin || (CustomMainPagePlugin = {}));
CustomMainPagePlugin.load();
