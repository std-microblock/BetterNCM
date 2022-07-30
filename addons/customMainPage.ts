namespace BetterNCMApi {
    export namespace fs {
        export async function readFile(path: string) {
            return await (
                await fetch(
                    "http://music.163.com/betterncm_api/read_file?" + path
                )
            ).text()
        }
        export async function writeFile(path: string, content: string) {
            await (
                await fetch(
                    "http://music.163.com/betterncm_api/write_file?" + path,
                    { body: content, method: "POST" }
                )
            ).text();
        }
    }
}

namespace CustomMainPagePlugin {
    function loadHTML(html: string) {
        document.querySelector(".g-mn.one div")!.innerHTML = html
    }
    function parseContent(pageContent: any) {
        let dom = document.createElement(pageContent.base||"div");

        dom.onclick = () => {
            parseActionBlocks(pageContent.action?.click);
        }

        switch (pageContent.type) {
            case "text": {
                dom.innerText = pageContent.text
                dom.style.fontSize = pageContent.size
                dom.style.padding = pageContent.padding
                dom.style.all = pageContent.style
                break;
            }
            case "br": {
                dom = document.createElement("br")
                break;
            }
            case "playlist":{
                dom.innerHTML=`<div style="width:150px" class="cvr u-cover u-cover-daily u-cover-recmd"  data-res-menu="true"  data-res-type="21">
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
                </div>`
            }
        }
        return dom
    }
    async function parseBlock(block: any) {
        switch (block.type) {
            case "renamePage": {
                document.querySelector("nav .firstlist a")!.innerHTML = block.name
                break;
            }
            case "setPageContent": {
                let a: ElementCreationOptions
                let generated = document.createElement("div");
                generated.classList.add("cc-microblock-betterncm-pagecontent")
                for (let content of block.content) {
                    generated.appendChild(parseContent(content));
                }
                document.querySelector(".g-mn.one")!.children[0]?.remove()
                document.querySelector(".g-mn.one")!.appendChild(generated)
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
    function parseActionBlocks(data: any[]) {
        for (let block of data) {
            parseBlock(block)
        }
    }
    let handle=-1;
    export async function load() {
        let data: any[] = JSON.parse(await BetterNCMApi.fs.readFile("./customMainPage.json"));
        clearInterval(handle)
        handle=setInterval(() => {
            if (document.querySelector(".g-mn.one .g-recmdwrap")) {
                parseActionBlocks(data);
            } else if (document.querySelectorAll(".g-mn.one>div").length > 1){
                let pageContentDom=document.querySelector(".cc-microblock-betterncm-pagecontent");
                if (pageContentDom) pageContentDom.remove()
            }

        }, 100)
    }

}

CustomMainPagePlugin.load()