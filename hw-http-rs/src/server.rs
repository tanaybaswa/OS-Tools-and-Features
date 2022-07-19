use std::env;

use crate::args;

use crate::http::*;
use crate::stats::*;

use clap::Parser;
use tokio::net::TcpStream;
use tokio::net::TcpListener;
use tokio::fs::File;
use tokio::io::{self, AsyncReadExt};
use tokio::fs;

use anyhow::Result;

pub fn main() -> Result<()> {
    // Configure logging
    // You can print logs (to stderr) using
    // `log::info!`, `log::warn!`, `log::error!`, etc.
    env_logger::Builder::new()
        .filter_level(log::LevelFilter::Info)
        .init();

    // Parse command line arguments
    let args = args::Args::parse();

    // Set the current working directory
    env::set_current_dir(&args.files)?;

    // Print some info for debugging
    log::info!("HTTP server initializing ---------");
    log::info!("Port:\t\t{}", args.port);
    log::info!("Num threads:\t{}", args.num_threads);
    log::info!("Directory:\t\t{}", &args.files);
    log::info!("----------------------------------");

    // Initialize a thread pool that starts running `listen`
    tokio::runtime::Builder::new_multi_thread()
        .enable_all()
        .worker_threads(args.num_threads)
        .build()?
        .block_on(listen(args.port))
}

async fn listen(port: u16) -> Result<()> {
    // Hint: you should call `handle_socket` in this function.
    let listener = TcpListener::bind(format!("0.0.0.0:{}", port)).await?;

    loop {
        let (socket, _) = listener.accept().await?;

        tokio::spawn(async move {
            // Process each socket concurrently.
            handle_socket(socket).await
        });
    }
}

// Handles a single connection via `socket`.
async fn handle_socket(mut socket: TcpStream) -> Result<()> {

    //get filepath from socket, using function already given.
    //append a '.'
    //check if it exists, if it doesnt, 404
    //get mime type.
    //read file to buffer and write to socket in a loop
    let mut buffer = [0; 1024];

    let _request = parse_request(&mut socket).await.unwrap();
    let mut path = format!("{}{}", ".", _request.path);


    let mut f = File::open(&mut path).await;
    
    let f = match f {
        Ok(file) => file,
        Err(e) => {
            start_response(&mut socket, 404);
            end_headers(&mut socket);
            return Ok(())
        }
    };

    let metadata = fs::metadata(&path).await.unwrap();
    let len =  metadata.len();
    let mut strlen = format!("{}", len);
    

    start_response(&mut socket, 200);
    send_header(&mut socket, "Content-Type", get_mime_type(&path));
    send_header(&mut socket, "Content-Length", &strlen);
    end_headers(&mut socket);
    /** 
    loop:
    read file into buffer.
    write buffer into socket.
    when done:
    close file, Ok(())

    */
 
    
    Ok(())   
}

// You are free (and encouraged) to add other funtions to this file.
// You can also create your own modules as you see fit.
