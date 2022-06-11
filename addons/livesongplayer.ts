var lsp_lastsongname="_";

(async ()=>{
    let t=await (await fetch("http://localhost:10103/")).text()
alert("网易云*点歌姬开始运行")
    function playSong(id) {
        document.querySelector(".j-search-input").value = id
        document.querySelector(".sch-btn").click()
        var lsp_pl_handle=setInterval(function () {
            if(!document.querySelector(".m-search .j-item"))return;
            playAllSong()
            clearInterval(lsp_pl_handle)
            setTimeout(function(){
                if(document.querySelector(".j-title"))
                lsp_lastsongname=document.querySelector(".j-title").innerText
                lsp_polling=false;
                play()
             //   if(document.querySelector(".fmdelete"))playAllSong()
                
            },300);
            
        },100)
        
    } 
    
    var lsp_polling=false;
    function poll_next_song(){
        if(lsp_polling)return;
        lsp_polling=true;
        fetch("http://localhost:10103/poll_song")
            .then(function(v){return v.json()})
            .then(function(t){
                playSong(t.id)
            })
    }
    
    setInterval(function(){
        fetch("http://localhost:10103/sync?__time"+getPlayTimePercent()+"__")
            .then(function(v){return v.text()})
            .then(function(t){
                if(t=="skip")poll_next_song()
            })
    },100);
    
    setInterval(function(){
        if(lsp_polling)return;
        
        if(getPlayTimePercent()>=1||lsp_lastsongname!=document.querySelector(".j-title").innerText){
            lsp_lastsongname=""
            poll_next_song();
            pause()
        }
    },400)
    
    function playAllSong() {
        document.querySelector(".m-search.q-lrc.g-wrap1 .container.j-flag .oper.g-wrap5.j-flag .u-ibtn5b.u-ibtn5b-new.j-oper .u-ibtn5-ply").click()
    }
    
    function pause(){
        document.querySelector(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp-pause").click();
    }
    
    function play(){
        document.querySelector(".f-fs0 #x-g-mn .m-player.f-os#main-player .btnp.btnp-play.f-cp").click();
    }
    
    function formatTime(time){
        return parseInt(time.split(":")[0]*60)+parseInt(time.split(":")[1]);
    }
    
    function getPlayTimePercent(){
        return formatTime(document.querySelector("time.now").innerText)/formatTime(document.querySelector("time.all").innerText)
    }
})()

