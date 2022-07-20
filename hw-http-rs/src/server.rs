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
use tokio::io::AsyncWriteExt;
use tokio::fs::ReadDir;
use std::ffi::OsString;

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
    let mut case = 0;

    let _request = parse_request(&mut socket).await.unwrap();
    let mut path = format!("{}{}", ".", _request.path);

    let metadata = fs::metadata(&path).await;

    let mut meta = match metadata {
        Ok(meta) => meta,
        Err(err) => {
            start_response(&mut socket, 404).await.unwrap();
            end_headers(&mut socket).await.unwrap();
            return Ok(())
        }
    };

    if meta.is_dir() {
        case = 1;
    } else {
        case = 0;
    }

    if case == 0 {
        let len =  meta.len();
        let mut strlen = format!("{}", len);
        

        let mut f = File::open(&mut path).await;
        
        let mut _f = match f {
            Ok(file) => file,
            Err(_e) => {
                start_response(&mut socket, 404).await.unwrap();
                end_headers(&mut socket).await.unwrap();
                return Ok(())
            }
        };
        

        start_response(&mut socket, 200).await.unwrap();
        send_header(&mut socket, "Content-Type", get_mime_type(&path)).await.unwrap();
        send_header(&mut socket, "Content-Length", &strlen).await.unwrap();
        end_headers(&mut socket).await.unwrap();
        

        while (_f.read(&mut buffer).await.unwrap()) > 0{
            socket.write_all(&mut buffer).await.unwrap();
        }

        socket.write_all(&mut buffer).await.unwrap();

    } else {
        let mut indexpath = format_index(&path);
        let imetadata = fs::metadata(&indexpath).await;

        let mut imeta = match &imetadata {
            Ok(meta) => 0,
            Err(err) => 1
        };

        if imeta == 0{

            let m = imetadata.unwrap();

            let len =  m.len();
            let mut strlen = format!("{}", len);
            

            let mut f = File::open(&mut indexpath).await;
            
            let mut _f = match f {
                Ok(file) => file,
                Err(_e) => {
                    start_response(&mut socket, 404).await.unwrap();
                    end_headers(&mut socket).await.unwrap();
                    return Ok(())
                }
            };
            

            start_response(&mut socket, 200).await.unwrap();
            send_header(&mut socket, "Content-Type", "text/html").await.unwrap();
            send_header(&mut socket, "Content-Length", &strlen).await.unwrap();
            end_headers(&mut socket).await.unwrap();
            

            while (_f.read(&mut buffer).await.unwrap()) > 0{
                socket.write_all(&mut buffer).await.unwrap();
            }

            socket.write_all(&mut buffer).await.unwrap();


        } else {
            
            start_response(&mut socket, 200).await.unwrap();
            send_header(&mut socket, "Content-Type", get_mime_type(&path)).await.unwrap();
            send_header(&mut socket, "Content-Length", "0").await.unwrap();
            end_headers(&mut socket).await.unwrap();

            let mut iter = tokio::fs::read_dir(&path).await.unwrap();

            loop {
                match iter.next_entry().await.unwrap() {
                    Some(entry) => {

                        let es = entry.file_name().into_string().unwrap();
                        let link = format_href(&path, &es);
                        socket.write_all((&link).as_bytes()).await.unwrap();
                    },
                    None => { break }
                }
            };
            let link = format_href(&path, "");
            socket.write_all((&link).as_bytes()).await.unwrap();
            
        }


    }
 
    
    Ok(())   
}

// You are free (and encouraged) to add other funtions to this file.
// You can also create your own modules as you see fit.


/*

New logic:

parse request.
get path.
if path is dir? dir code.
else? file code. 

dir code:












*/