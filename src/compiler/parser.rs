use super::ast::*;
use self::Keyword::*;
use self::ExprType::*;
use self::TokenType::*;
use super::lexer::Lexer;

pub struct Parser<'a> {
    source: &'a str,
    context: &'a str,
    lexer: Lexer<'a>,
}

impl<'a> Parser<'a> {
    pub fn new(context: &'a str, source: &'a str) -> Parser<'a> {
        Parser {
            source: source,
            context: context,
            lexer: Lexer::new(source),
        }
    }

    #[inline]
    fn format_error(&self, (message, (column, line, start)): ParserError) -> String {
        format!("Error on {}:{}:{}> {}\n  {}",
            self.context, line, column, message,
            self.source
                .get(start..).unwrap_or("")
                .lines().next().unwrap_or("")
                .trim(),
        )
    }

    pub fn parse(&mut self) -> Result<List<Expr>, String> {
        Err(String::new())
    }
}
