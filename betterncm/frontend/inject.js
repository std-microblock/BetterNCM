
var addons;
var reloadInject = {}
function main() {
    "use strict";
    fetch("https://music.163.com/betterncm_api/addons").then(function (resp) { return resp.text() }).then(function (resp1) {
        addons = JSON.parse(resp1);
        setInterval(function () {
            for (var x = 0; x < addons.length; x++) {
                var addon_892k = addons[x];
                if(addon_892k.enabled)
                if (addon_892k.devMode) {
                    fetch("https://music.163.com/betterncm_api/read_file?" + addon_892k.file)
                        .then(function (resp_2856) { return resp_2856.text() })
                        .then(function (resp2) {
                            if (reloadInject[addon_892k.file] && resp2 != reloadInject[addon_892k.file]) {
                                document.location.reload()
                            }
                        })
                }
            }
        }, 1000);
        for (var x = 0; x < addons.length; x++) {
            var addon = addons[x];
            if(addon.enabled)
            fetch("https://music.163.com/betterncm_api/read_file?" + addon.file)
                .then(function (resp) { return resp.text() })
                .then(function (resp) {
                    reloadInject[addon.file] = resp
                    var script = document.createElement("script");
                    script.innerHTML = resp;
                    document.body.appendChild(script);
                })
        }
    });
}
main()
