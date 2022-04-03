var cssLoaderLiveReload={}
fetch("https://music.163.com/betterncm_api/read_file?stylesheets/all.json")
.then(function(resp){return resp.json()})
.then(function(resp){
    for(var x=0;x<resp.length;x++){
        var style=resp[x];
        var link=document.createElement("link");
        link.rel="stylesheet";
        link.href="https://music.163.com/betterncm_api/read_file?stylesheets/"+style.file;
        document.head.appendChild(link);
        if(style.devMode){
            
            setInterval(function(){
                fetch("https://music.163.com/betterncm_api/read_file?stylesheets/"+style.file)
                .then(function(resp){return resp.text()})
                .then(function(resp){
                    if(!cssLoaderLiveReload[style.file])
                        cssLoaderLiveReload[style.file]=resp
                    
                    if(resp!=cssLoaderLiveReload[style.file])document.location.reload()
                })
            },1000);
        }
    }
})