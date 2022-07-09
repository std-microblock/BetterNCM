"use strict";

var addons;
var reloadInject = {};

function main() {
  "use strict";

  window["__injected__"] = true;
  fetch("https://music.163.com/betterncm_api/getdir?").then(function (resp) {
    return resp.text();
  }).then(function (resp1) {
    addons = resp1.split(",");

    for (var x = 0; x < addons.length; x++) {
      var addon = addons[x];
      if (!addon.endsWith(".js")) continue;
      fetch("https://music.163.com/betterncm_api/read_file?" + addon).then(function (resp) {
        return resp.text();
      }).then(function (resp) {
        var script = document.createElement("script");
        script.innerHTML = resp.replace(/localhost\/betterncm_api\//g, "music.163.com/betterncm_api/");
        document.body.appendChild(script);
      });
    }
  });
}

if (!window["__injected__"]) main();