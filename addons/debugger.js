setInterval(function(){
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
