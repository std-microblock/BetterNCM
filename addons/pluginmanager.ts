var onlinePlugins: Array<Object> = [];

var postInstall = async function (ind: number) {
  let plugin = onlinePlugins[ind];
  let content = await (await fetch(plugin["url"])).text();
  try {
    await (
      await fetch(
        "http://localhost/betterncm_api/write_file?" + plugin["filepath"],
        { body: content, method: "POST" }
      )
    ).text();
  } catch (e) {
    alert(e);
  }
  document.location.reload();
};

var postUninstall = async function (ind: number) {
  let plugin = onlinePlugins[ind];
  await (
    await fetch(
      "http://localhost/betterncm_api/write_file?" + plugin["filepath"],
      { body: "", method: "POST" }
    )
  ).text();
  document.location.reload();
};

(async () => {
  let handle: number;

  try {
    onlinePlugins = await (
      await fetch(
        "https://ganbei-hot-update-1258625969.file.myqcloud.com/betterncm/online_addons.json?" +
          new Date().getTime()
      )
    ).json();
  } catch (e) {}
  for (let plugin of onlinePlugins) {
    let script;
    try {
      script = await (
        await fetch(
          "http://localhost/betterncm_api/read_file?" + plugin["filepath"]
        )
      ).text();
    } catch (e) {
      script = "";
    }
    // alert(script)
    if (script != "") plugin["installed"] = true;
  }

  handle = setInterval(function () {
    let ele = document.querySelector(".g-mn-set");
    if (ele && !document.querySelector("#betterncmPM")) {
      let settings = document.createElement("div");
      settings.style.padding = "20px";
      settings.style.width = "100%";

      settings.style.zIndex = "100000";
      settings.id = "betterncmPM";
      settings.innerHTML = `
      <h3 class='s-fc1 f-ff2' style='font-size:24px;font-weight:800;margin:20px;'>BetterNCM</h3>
      <button class='u-ibtn5' 
      onclick=' fetch(\"https://localhost/betterncm_api/opensettings\").then(function(){document.location.reload()})'>
      编辑设置
      </button>
      
      <button class='u-ibtn5' onclick='fetch(\"https://localhost/betterncm_api/opencsssettings\")
      .then(function(){document.location.reload()})'>编辑CSS设置</button>
      
      <button class='u-ibtn5' onclick='fetch(\"https://localhost/betterncm_api/openconfigfolder\")'>
      打开配置文件夹</button>
      
      <button class='u-ibtn5' onclick='document.location.reload()'>重载页面</button>
      
      <h3 class='s-fc1 f-ff2' style='font-size:19px;font-weight:700;margin:10px;'>BetterNCM 插件列表</h3>
      <div class="onlinepluginlish">
          ${onlinePlugins
            .map((plugin, ind) => {
              return `<div>
                <span style="font-size:16px;font-weight:600;">${plugin["name"]}</span>
                <span style="font-size:14px;font-weight:500;color:gray;">(${plugin["author"]})</span>
                <button class='u-ibtn5' style="display:${
                  plugin["installed"] ? "none" : "inline-block"
                };" onclick="postInstall(${ind});">Install</button>
                <button class='u-ibtn5' style="display:${
                  plugin["installed"] ? "inline-block" : "none"
                };" onclick="postUninstall(${ind});">Uninstall</button>
                <div style="padding:5px;">
                ${plugin["description"]}
                </div>
            </div><br>`;
            })
            .join("")}
      </div>`;
      ele.appendChild(settings);
    }
  }, 100);
})();
