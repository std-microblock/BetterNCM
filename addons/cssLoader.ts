interface BetterNCMStyleSheet {
  generator(config: Map<string, string>): string;
  get_configs(): Array<string>;
  get_name(): string;
}

window["configs"] = JSON.parse(
  localStorage["betterncm.cssloader.config"] || "{}"
);

window["saveCSSSettings"]=function(){
  localStorage["betterncm.cssloader.config"]=JSON.stringify(window["configs"])
  document.location.reload()
}

class CSSLoader {
  static async loadStyles() {
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
      configs[styleObj.get_name()]=configs[styleObj.get_name()]||styleObj.get_configs()

      configsHTML += `
        <div>
            <h3 style='font-size:16px;font-weight:700;'>${styleObj.get_name()}</h3>
            ${Object.keys(styleObj.get_configs()).map((v) => {
              return `<div>${v}:
              <input class="txt u-txt __cssLoader__config__input__" 
              onkeyup='window["configs"]["${styleObj.get_name()}"]["${v}"]=event.target.value'
              value="${configs[styleObj.get_name()][v]}"></div>`;
            })}
            
        </div>
      `;

      setInterval(() => {
        let pm = document.querySelector("#betterncmPM");
        let cssConfig = document.querySelector("#betterncmCSSConf");
        if (pm && !cssConfig) {
          let conf = document.createElement("div");
          conf.id = "betterncmCSSConf";
          conf.innerHTML = configsHTML;
          pm.appendChild(conf);
        }
      }, 100);

      let style = document.createElement("style");
      style.innerHTML = styleObj.generator(configs[styleObj.get_name()]);
      document.head.appendChild(style);
    }
    configsHTML+=`<button class='u-ibtn5' onclick="saveCSSSettings()">确定</button>`
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
            _text=_text.replace(`[[${config}]]`, configValue);
        }
        return _text;
      },
      get_configs() {
        return ssjson["configs"];
      },
      get_name() {
        return ssjson["name"];
      },
    };
  }
}

CSSLoader.loadStyles();
