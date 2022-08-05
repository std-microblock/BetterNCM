use crate::{api::*, config::config_path};
use warp::{Filter, path::param};

pub fn start_webserver(){
    tokio::spawn(async {
        let api_root=warp::path::path("api");

        let api=
            api_root.and(warp::path::path("openFileSelectDialog")).and(param().and_then(file_select_dialog))
             .or(api_root.and(warp::path::path("openFileSelectDialog")).and(param().and_then(file_select_dialog)));

        let file=warp::fs::dir(config_path());

        warp::serve(api.or(file))
            .run(([127, 0, 0, 1], 3297))
            .await;
    });
}