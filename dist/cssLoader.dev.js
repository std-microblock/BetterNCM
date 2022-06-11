"use strict";

var cssLoaderLiveReload = {};
fetch("https://localhost/betterncm_api/read_file?stylesheets/all.json").then(function (resp) {
  return resp.json();
}).then(function (resp) {
  setInterval(function () {
    for (var x = 0; x < resp.length; x++) {
      var style_23ujd = resp[x];

      if (style_23ujd.enabled && style_23ujd.devMode) {
        fetch("https://localhost/betterncm_api/read_file?stylesheets/" + style_23ujd.file).then(function (resp) {
          return resp.text();
        }).then(function (resp) {
          if (!cssLoaderLiveReload[style_23ujd.file]) cssLoaderLiveReload[style_23ujd.file] = resp;
          if (resp != cssLoaderLiveReload[style_23ujd.file]) document.location.reload();
        });
      }
    }
  }, 1000);

  for (var x = 0; x < resp.length; x++) {
    var style = resp[x];
    if (!style.enabled) continue;
    var link = document.createElement("link");
    link.rel = "stylesheet";
    link.href = "https://localhost/betterncm_api/read_file?stylesheets/" + style.file;
    document.head.appendChild(link);
  }
});