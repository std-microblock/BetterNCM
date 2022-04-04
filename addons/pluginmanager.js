setInterval(function(){
    if(document.querySelector(".g-mn-set")&&!document.querySelector("#betterncmPM")){
        var settings=document.createElement("div")
        // settings.style.position="absolute"
        // settings.style.left="10px"
        settings.style.padding="20px"
        settings.style.width="100%"

        settings.style.zIndex="100000"
        settings.id="betterncmPM"
        // settings.style.background="yellow"
        settings.innerHTML="<h2 class='s-fc1 f-ff2'>BetterNCM</h2><button onclick='fetch(\"https://music.163.com/betterncm_api/opensettings\").then(function(){document.location.reload()})'>编辑设置</button><button onclick='fetch(\"https://music.163.com/betterncm_api/opencsssettings\").then(function(){document.location.reload()})'>编辑CSS设置</button><button onclick='fetch(\"https://music.163.com/betterncm_api/openconfigfolder\")'>打开配置文件夹</button><button onclick='document.location.reload()'>重载页面</button>"
        document.querySelector(".g-mn-set").appendChild(settings)
    }
},100)