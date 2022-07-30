"use strict";

!function () {
  function getPlayingID() {
    document.querySelector("*[data-res-action=opencomment]").dataset["resId"];
  }

  document.querySelector(".btn-dld").addEventListener("contextmenu", function (e) {
    fetch("https://music.163.com/betterncm_api/open?http://music.163.com/song/media/outer/url?id=".concat(getPlayingID(), ".mp3"));
    e.preventDefault();
  });
}();