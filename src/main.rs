

#[macro_use] extern crate lazy_static;

#[allow(dead_code)] mod compiler;

use compiler::lexer::Lexer;

fn main() {
    for (token, _pos) in Lexer::new("x = 5") {
        println!("Found: {:?}", token)
    }
}