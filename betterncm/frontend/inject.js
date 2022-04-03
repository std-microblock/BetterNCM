var addons;
function main(){
    fetch("https://music.163.com/betterncm_api/addons").then(function(resp){return resp.text()}).then(function(resp){
        addons = JSON.parse(resp);
        for(var x=0;x<addons.length;x++){
            var addon=addons[x];
            fetch("https://music.163.com/betterncm_api/read_file?"+addon.file)
            .then(function(resp){return resp.text()})
            .then(function(resp){
                if(addon.devMode){
                    setInterval(function(){
                        fetch("https://music.163.com/betterncm_api/read_file?"+addon.file)
                        .then(function(resp){return resp.text()})
                        .then(function(respNow){
                            if(resp!=respNow)document.location.reload()
                        })
                    },1000);
                }
                var script=document.createElement("script");
                script.innerHTML=resp;
                document.body.appendChild(script);
            })
        }
    });
}
main()
