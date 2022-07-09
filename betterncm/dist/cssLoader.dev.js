"use strict";

fetch("https://music.163.com/betterncm_api/getdir?stylesheets/").then(function (resp) {
  return resp.text();
}).then(function (resp) {
  resp = resp.split(',');

  for (var x = 0; x < resp.length; x++) {
    var style = resp[x];
    var link = document.createElement("link");
    link.rel = "stylesheet";
    link.href = "https://music.163.com/betterncm_api/read_file?stylesheets/" + style;
    document.head.appendChild(link);
  }
});