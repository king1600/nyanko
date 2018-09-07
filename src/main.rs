

#[macro_use] extern crate lazy_static;

#[allow(dead_code)] mod compiler;

use compiler::lexer::Lexer;

fn main() {
    let mut lexer = Lexer::new("x = 5");
    while let Ok(Some((token, _pos))) = lexer.next() {
        println!("Found {:?}", token);
    }
}