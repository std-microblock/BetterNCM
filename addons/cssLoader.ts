interface BetterNCMStyleSheet {
  generator(config: Map<string, string>): string;
  get_configs(): Array<string>;
  get_name(): string;
  is_debug(): boolean;
}

window["configs"] = JSON.parse(
  localStorage["betterncm.cssloader.config"] || "{}"
);

window["saveCSSSettings"] = function () {
  localStorage["betterncm.cssloader.config"] = JSON.stringify(window["configs"])
  setTimeout(() => {
    CSSLoader.loadStyles();
  }, 100)
  // document.location.reload()
}

let liveReloadHandles: { [key: string]: number } = {}
let liveReloadLastValues: { [key: string]: string } = {}

let lastHandle = -1;

class CSSLoader {
  static async loadStyles() {
    clearInterval(lastHandle)
    document.querySelectorAll("style.cc-microblock-cssloader-style").forEach(e => e.remove())
    document.querySelector("#betterncmCSSConf")?.remove()

    let stylesheetsText: string = await (
      await fetch("https://music.163.com/betterncm_api/getdir?stylesheets/")
    ).text();
    let stylesheets = stylesheetsText.split(",");

    let configsHTML = `<h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM StyleSheet设置</h3>`;

    for (let stylesheetFile of stylesheets) {
      if (!stylesheetFile.endsWith(".css")) continue;
      let url =
        "https://music.163.com/betterncm_api/read_file?stylesheets/" +
        stylesheetFile;

      let stylesheet = await (await fetch(url)).text();
      let styleObj = this.parseStyle(stylesheet);
      configs[styleObj.get_name()] = configs[styleObj.get_name()] || styleObj.get_configs()

      configsHTML += `
        <div>
            <h3 style='font-size:16px;font-weight:700;'>${styleObj.get_name()}</h3>
            ${Object.keys(styleObj.get_configs()).map((v) => {
        console.log(configs[styleObj.get_name()][v])
        return `<div>${v}:
              <input class="txt u-txt __cssLoader__config__input__" 
              onkeyup='window["configs"]["${styleObj.get_name()}"]["${v}"]=event.target.value'
              value="${configs[styleObj.get_name()][v]}"></div>`;
      })}
            
        </div>
      `;

      // Live Reload
      {
        if (styleObj.is_debug()) {
          if (!liveReloadHandles[styleObj.get_name()]) {
            liveReloadLastValues[styleObj.get_name()] = stylesheet;

            liveReloadHandles[styleObj.get_name()] = setInterval(async () => {
              let url =
                "https://music.163.com/betterncm_api/read_file?stylesheets/" +
                stylesheetFile;

              let stylesheet = await(await fetch(url)).text();

              if(stylesheet!=liveReloadLastValues[styleObj.get_name()]){
                liveReloadLastValues[styleObj.get_name()]=stylesheet
                await this.loadStyles()
              }
            }, 400)
          }

        }
      }





      let style = document.createElement("style");
      style.classList.add("cc-microblock-cssloader-style")
      style.innerHTML = styleObj.generator(configs[styleObj.get_name()]);
      document.head.appendChild(style);
    }
    configsHTML += `<button class='u-ibtn5' onclick="saveCSSSettings()">确定</button>`

    lastHandle = setInterval(() => {
      let pm = document.querySelector("#betterncmPM");
      let cssConfig = document.querySelector("#betterncmCSSConf");
      if (pm && !cssConfig) {
        let conf = document.createElement("div");
        conf.id = "betterncmCSSConf";
        conf.innerHTML = configsHTML;
        pm.appendChild(conf);
      }
    }, 100);
  }

  static parseStyle(stylesheetText: string): BetterNCMStyleSheet {
    let stylejson = /==\|\|((.|\n|\r)*)\|\|==/.exec(stylesheetText);
    if (
      !((stylesheetText.includes("[[ BetterNCM-CSS-V2 ]]") ||
        stylesheetText.includes("[[BetterNCM-CSS-V2]]")) &&
        stylejson)
    )
      return {
        generator(config) {
          return stylesheetText;
        },
        get_configs() {
          return [];
        },
        get_name() {
          return "Unnamed Stylesheet";
        },
        is_debug() {
          return false
        }
      };

    let ssjson = JSON.parse(stylejson[1]);

    return {
      generator(cconfig) {
        let _text = stylesheetText;
        let configs: Array<string> = this.get_configs();
        for (let config of Object.keys(configs)) {
          let configValue = cconfig[config];
          if (configValue)
            while (_text.includes(`[[${config}]]`))
              _text = _text.replace(`[[${config}]]`, configValue);
        }
        return _text;
      },
      get_configs() {
        return ssjson["configs"];
      },
      get_name() {
        return ssjson["name"];
      }, is_debug() {
        return ssjson["debug"] || false;
      }
    };
  }
}

CSSLoader.loadStyles();
