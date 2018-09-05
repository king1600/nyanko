

#[macro_use] extern crate lazy_static;

#[allow(dead_code)] mod compiler;

use compiler::lexer::Lexer;

fn main() {
    for (token, pos) in Lexer::new("5.14".as_bytes()) {
        println!("Found: {:?}", token)
    }
}