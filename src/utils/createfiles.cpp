#include "pch.h"
#include "createfiles.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
// By skykey
const char* addonsJsonData =
R"([{"name":"cssLoader","file":"cssLoader.js","enabled":true,"devMode":false},{"name":"debugger","file":"debugger.js","enabled":false,"devMode":false},{"name":"PluginManager","file":"pluginmanager.js","enabled":true,"devMode":false}])";
const char* cssLoaderJsData =
R"(fetch("http://localhost:3248/read_file?filename=stylesheets/all.json")
    .then(function (resp) { return resp.json() })
    .then(function (resp) {
       
        for (var x = 0; x < resp.length; x++) {
            var style = resp[x];
            if (!style.enabled) continue;
            fetch("http://localhost:3248/read_file?filename=stylesheets/"+style.file)
                .then(function (resp) { return resp.text() })
                .then(function (resp) {
                    var style = document.createElement('style');
                    style.innerHTML = resp;
                    document.head.appendChild(style);
                })
        }
    }))";
const char* debuggerJsData = R"(setInterval(function(){
    if(document.querySelector("#debuggingbarDiv"))return
    var debuggingbar=document.createElement("div")
    debuggingbar.style.position="absolute"
    debuggingbar.style.left="10px"
    debuggingbar.style.top="10px"
    debuggingbar.style.zIndex="100000"
    debuggingbar.id="debuggingbarDiv"
    debuggingbar.style.background="yellow"
    debuggingbar.innerHTML="<button id='reloadBtn' onclick='document.location.reload()'>Reload</button>"
    document.body.appendChild(debuggingbar)
},1000)

var debuggerVars={}

function copyTextToClipboard(text) {
    var textArea = document.createElement("textarea");
    textArea.value = text;
    
    // Avoid scrolling to bottom
    textArea.style.top = "0";
    textArea.style.left = "0";
    textArea.style.position = "fixed";
  
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();
  
    try {
      var successful = document.execCommand('copy');
      var msg = successful ? 'successful' : 'unsuccessful';
      console.log('Fallback: Copying text command was ' + msg);
    } catch (err) {
      console.error('Fallback: Oops, unable to copy', err);
    }
  
    document.body.removeChild(textArea);
  }

addEventListener("mousemove",function(e){
    // this.alert(Object.keys(e).join("\n"))
    debuggerVars["lastpath"]=e.toElement
})
function getStr(obj){
    if(!obj||obj.tagName=="html")return "-"
    return getStr(obj.parentElement)+" > \n"+obj.tagName+"."+obj.classList.toString().split(" ").join(".")+"#"+obj.id
}
addEventListener("keypress",function(e){
    if(e.keyCode==112){
        copyTextToClipboard(getStr(debuggerVars["lastpath"]))
        alert("[ Copied to clipboard ]\n"+getStr(debuggerVars["lastpath"]))
    }
    if(e.keyCode==101){
        this.alert(eval(this.prompt("Eval code:")))
    }
})
)";
const char* pluginManagerJsData = R"(setInterval(function () {
    if (document.querySelector(".g-mn-set") && !document.querySelector("#betterncmPM")) {
        var settings = document.createElement("div")
        // settings.style.position="absolute"
        // settings.style.left="10px"
        settings.style.padding = "20px"
        settings.style.width = "100%"

        settings.style.zIndex = "100000"
        settings.id = "betterncmPM"
        // settings.style.background="yellow"
        settings.innerHTML = "<h3 class='s-fc1 f-ff2' style='font-size:20px;font-weight:800;margin:20px;'>BetterNCM</h3><button class='u-ibtn5' onclick='fetch(\"http://localhost:3248/explorer\")'>打开配置文件夹夹夹夹夹夹夹夹夹夹夹夹</button><button class='u-ibtn5' onclick='document.location.reload()'>重载页面</button><div class='plugins'></div>"
        
        document.querySelector(".g-mn-set").appendChild(settings)
    }
}, 100)
function copyTextToClipboard(text) {
    var textArea = document.createElement("textarea");
    textArea.value = text;
    
    // Avoid scrolling to bottom
    textArea.style.top = "0";
    textArea.style.left = "0";
    textArea.style.position = "fixed";
  
    document.body.appendChild(textArea);
    textArea.focus();
    textArea.select();
  
    try {
      var successful = document.execCommand('copy');
      var msg = successful ? 'successful' : 'unsuccessful';
      console.log('Fallback: Copying text command was ' + msg);
    } catch (err) {
      console.error('Fallback: Oops, unable to copy', err);
    }
  
    document.body.removeChild(textArea);
  }
setInterval(function () {
    fetch("http://localhost:3248/addons").then(function (resp) { return resp.text() }).then(function (resp) {
        var addons = JSON.parse(resp);
        
        var generated = ""
        for (var x = 0; x < addons.length; x++) {
            var addon = addons[x];
            if (addon.enabled) {
                generated += "<div class='mod'><span>" + addon.file
                    + "</span><span class='bnbtn' onclick='uninstallmod(\"" + addon.file + "\")'>卸载</span>"
                    + "<span class='bnbtn' onclick='disablemod(\"" + addon.file + "\")'>禁用</span>"
                    + "</div>"
            }else{
                generated += "<div class='mod'><span>" + addon.file
                    + "</span><span class='bnbtn' onclick='uninstallmod(\"" + addon.file + "\")'>卸载</span>"
                    + "<span class='bnbtn' onclick='enablemod(\"" + addon.file + "\")'>启用</span>"
                    + "</div>"
            }
        }
        document.querySelector(".plugins").innerHTML = generated
    })
}, 100)

function uninstallmod(ui) {
    fetch("http://localhost:3248/addons").then(function (resp) { return resp.text() }).then(function (resp) {
    var addons = JSON.parse(resp);
    for (var n = 0; n < addons.length; n++) {
        if (addons[n].file == ui) {
            addons.splice(n, 1)
        }
    }
    fetch("http://localhost:3248/write_file?filename=addons.json",
        {
            method: "POST",
            body: JSON.stringify(addons),
            headers: { "Content-Type": "application/json" }
        }).then(function(){
            document.location.reload()
        });
    })
}
function disablemod(ui) {
    fetch("http://localhost:3248/addons").then(function (resp) { return resp.text() }).then(function (resp) {
        
    var addons = JSON.parse(resp);
    
    for (var n = 0; n < addons.length; n++) {
        if (addons[n].file == ui) {
            
            addons[n].enabled=false
        }
    }
    fetch("http://localhost:3248/write_file?filename=addons.json",
        {
            method: "POST",
            body: JSON.stringify(addons),
            headers: { "Content-Type": "application/json" }
        }).then(function(){
            document.location.reload()
        });
    })
}
function enablemod (ui) {
    fetch("http://localhost:3248/addons").then(function (resp) { return resp.text() }).then(function (resp) {
    var addons = JSON.parse(resp);
    for (var n = 0; n < addons.length; n++) {
        if (addons[n].file == ui) {
            addons[n].enabled=true
        }
    }
    fetch("http://localhost:3248/write_file?filename=addons.json",
        {
            method: "POST",
            body: JSON.stringify(addons),
            headers: { "Content-Type": "application/json" }
        }).then(function(){
            document.location.reload()
        });})
})";
const char* allJsonData = R"([
    {
        "file":"pluginmanager.css",
        "enabled":true
    },
    {
        "file":"block.css",
        "enabled":false
    },{
        "name":"Unbounded",
        "file":"Unbounded.css",
        "enabled":true
    }
])";
const char* blockCssData = R"(*{
    border-radius: 0px !important;
}

.btn,.cface,.ply,button{
    box-shadow: 2px 3px 6px rgba(0, 0, 0, 0.158),-2px 1px 4px rgba(0, 0, 0, 0.068);
}

.cdbox{
    opacity: 0 !important;
}

.cdwrap{
    rotate: 0 !important;
    -webkit-animation: none !important;
})";
const char* pluginmanagerCssData = R"(.plugins{
    padding: 10px;
    border-radius: 4px;
    margin: 10px;
    width: 200px;
    box-shadow: 0 0 10px rgba(0,0,0,0.2);
    background: #fff;
}

.plugins .mod{
    padding: 2px;
    border-radius: 4px;
    margin: 5px;
    background: #fff;
}

.plugins *{
    margin: 2px;
    color:black !important;
}

.plugins .mod span.bnbtn{
    padding: 3px;
    border-radius: 4px;
    cursor: pointer;
    border: black 1px solid;
})";
const char* UnboundedCssData = R"(#portal_root {
    background: url("file://G:/image.webp");
    background-size: cover;
    -webkit-filter: blur(5px) brightness(0.8);
    -webkit-transform: scale(1.1);
}

* {
    -webkit-transition: all 0.2s;
    scroll-behavior: smooth;
}

.m-nav *{
    color: white !important;
    text-shadow: 1px 1px 4px #00000077;
}


.z-tab-scroll,
.g-mn,
.n-set {
    background: transparent !important;
}


/* 滚动槽 */
::-webkit-scrollbar {
    display: none;
}


#main-player{
    background: rgba(0,0,0, 0);
}


#main-player {
    margin: 10px;
    border-radius: 10px;
    -webkit-transition: all 0.1s;
    border: none !important;
}

.nav{
    margin-bottom: 10px;
    /* -webkit-transform: translateY(-40px); */
    -webkit-mask-image: -webkit-linear-gradient(#000 95%, transparent);
    height: calc(100vh - 160px) !important;
    overflow: scroll;
}



.bar {
    left: 10px !important;
    bottom: 10px !important;
}

.m-tabwrap {
    background: rgba(255, 255, 255, 0) !important;
    position: absolute;
    top: 20px;
}

#x-g-mn,
.z-show {
    overflow: hidden;
    border-radius: 5px;
}

.g-sd {
    border-right: none !important;
}


.mq-playing .g-mn.one,
.mq-playing .m-nav {
    opacity: 0;
}
.g-singlec-ct {
    /* position: absolute; */
    top: 20px !important;
    /* -webkit-transform: translateY(-40px); */
    /* margin-top: -40px; */
}
.n-single .wrap {
    margin: 0 !important;
    /* -webkit-transform: translateY(-40px); */
}

.n-single {}

/* .n-single .wrap,.g-singlec-ct */
.bg.j-flag {
    display: none;
}

.g-single {
    position: absolute;
    top: 50px;
    /* border: 1px rgba(0, 0, 0, 0.219) solid; */
    /* border-radius: 10px; */
    padding: 0 !important;
    margin: 20px 10px !important;
    background: transparent;
    /* background: rgba(0, 0, 0, 0.404); */
    /* -webkit-transform: translateY(25px); */
}

/* white theme lyric  */
body.s-theme-white .nav svg {
    -webkit-filter: invert(100%) sepia(0%) saturate(7486%) hue-rotate(144deg) brightness(99%) contrast(104%);
}

body.s-theme-white * {
    color:white !important;
}
.g-hd {
    background: rgba(35, 35, 38, 0) !important;
    border: none !important;
}


body:not(.s-theme-white) .g-mn {
    background: rgba(0, 0, 0, 0) !important;
    
}

header::after{
    background: none !important;
    border: none !important;
}


.g-mn.one {
    /* padding: 4px; */

    -webkit-transform: translateY(-5px);
    margin: 10px;
    border-radius: 10px;

}


.fx.j-flag.fsection {
    /* background: rgba(255, 122, 177, 0.438); */
}

.j-flag {
    background: unset;
}

.nav li {
    margin: 5px;
    border-radius: 5px;
}

.nav li:hover {
    background: rgba(0, 0, 0, 0);
    -webkit-transform: scale(1.01);
}

/* Remove ADs */
.livewrap,
.voicelive,
.m-list-recmd-live,
.m-banner,
.heartbeat,
.icn-live-sm,
.u-icn-novip {
    display: none !important;
}

.nav .firstlist>*:nth-child(2) {
    display: none;
}

.nav .firstlist>*:nth-child(3) {
    display: none;
}

.nav .firstlist>*:nth-child(4) {
    display: none;
}

.nav .firstlist>*:nth-child(5) {
    display: none;
}

.nav .firstlist>*:nth-child(6) {
    display: none;
})";

int createBetterNCMFiles() {
    std::string ncmStrPath{ std::string(getenv("USERPROFILE")) + "/betterncm" };
    std::filesystem::path ncmPath(ncmStrPath);

    if (std::filesystem::exists(ncmPath)) {
        std::cout << "Bye..." << std::endl;
        return 0;
    }
    else {
        std::cout << "Creating..." << std::endl;
        std::filesystem::create_directories(ncmPath / "stylesheets");

        std::ofstream addonsJson(ncmPath / "addons.json");
        addonsJson << addonsJsonData;

        std::ofstream cssLoaderJs(ncmPath / "cssLoader.js");
        cssLoaderJs << cssLoaderJsData;

        std::ofstream debuggerJs(ncmPath / "debugger.js");
        debuggerJs << debuggerJsData;

        std::ofstream pluginmanagerJs(ncmPath / "pluginmanager.js");
        pluginmanagerJs << pluginManagerJsData;

        ncmPath /= "stylesheets";
        std::ofstream allJson(ncmPath / "all.json");
        allJson << allJsonData;

        std::ofstream blockCss(ncmPath / "block.css");
        blockCss << blockCssData;

        std::ofstream pluginmanagerCss(ncmPath / "pluginmanager.css");
        pluginmanagerCss << pluginmanagerCssData;

        std::ofstream UnboundedCss(ncmPath / "Unbounded.css");
        UnboundedCss << UnboundedCssData;

        addonsJson.close();
        cssLoaderJs.close();
        debuggerJs.close();
        pluginmanagerJs.close();
        pluginmanagerCss.close();
        blockCss.close();
        UnboundedCss.close();
        allJson.close();

        std::cout << "Created!" << std::endl;
        return 0;
    }
    return 0;
}