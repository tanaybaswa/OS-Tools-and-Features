//! The gRPC client.
//!

use anyhow::Result;

// Client methods. DO NOT MODIFY THEIR SIGNATURES.
pub async fn example(input: u32) -> Result<u32> {
    todo!("TODO: Sending requests")
}
pub async fn echo(msg: String) -> Result<String> {
    todo!("TODO: Sending requests")
}
pub async fn put(key: Vec<u8>, value: Vec<u8>) -> Result<()> {
    todo!("TODO: KV store")
}
pub async fn get(key: Vec<u8>) -> Result<Vec<u8>> {
    todo!("TODO: KV store")
}
