pub async fn file_select_dialog(file_exts:String) -> Result<Box<dyn warp::Reply>, warp::Rejection> {
    use rfd::FileDialog;
    use crate::config::config_path;

    let files = FileDialog::new()
        .add_filter("File", &file_exts.split(",").collect::<Vec<_>>())
        .set_directory(config_path())
        .pick_file();

    if let Some(file) = files{
        Ok(Box::new(file.to_string_lossy().to_string()))
    }else{
        Ok(Box::new("Failed"))
    }
}
