
var addons;
var reloadInject = {}
function main() {
    "use strict";
    fetch("https://localhost/betterncm_api/getdir?").then(function (resp) { return resp.text() }).then(function (resp1) {
        addons = resp1.split(",");
        
        for (var x = 0; x < addons.length; x++) {
            var addon = addons[x];
            if(!addon.endsWith(".js"))continue;
            fetch("https://localhost/betterncm_api/read_file?" + addon)
                .then(function (resp) { return resp.text() })
                .then(function (resp) {
                    reloadInject[addon] = resp
                    var script = document.createElement("script");
                    script.innerHTML = resp;
                    document.body.appendChild(script);
                })
        }
    });
}
main()
