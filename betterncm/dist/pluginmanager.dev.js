"use strict";

setInterval(function () {
  if (document.querySelector(".g-mn-set") && !document.querySelector("#betterncmPM")) {
    var settings = document.createElement("div"); // settings.style.position="absolute"
    // settings.style.left="10px"

    settings.style.padding = "20px";
    settings.style.width = "100%";
    settings.style.zIndex = "100000";
    settings.id = "betterncmPM"; // settings.style.background="yellow"

    settings.innerHTML = "<h3 class='s-fc1 f-ff2' style='font-size:20px;font-weight:800;margin:20px;'>BetterNCM</h3><button class='u-ibtn5' onclick='fetch(\"https://localhost/betterncm_api/opensettings\").then(function(){document.location.reload()})'>编辑设置</button><button class='u-ibtn5' onclick='fetch(\"https://localhost/betterncm_api/opencsssettings\").then(function(){document.location.reload()})'>编辑CSS设置</button><button class='u-ibtn5' onclick='fetch(\"https://localhost/betterncm_api/openconfigfolder\")'>打开配置文件夹</button><button class='u-ibtn5' onclick='document.location.reload()'>重载页面</button>";
    document.querySelector(".g-mn-set").appendChild(settings);
  }
}, 100);