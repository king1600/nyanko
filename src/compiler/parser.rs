use super::ast::*;
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

    pub fn parse(&mut self) -> Result<List<Expr>, String> {
        let mut modules = List::new();
        while let Some(result) = self.parse_module() {
            match result {
                Ok(module) => modules.push_back(module),
                Err(error) => return Err(self.format_error(error)),
            }
        }
        Ok(modules)
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

    fn parse_module(&mut self) -> Option<Result<Expr, ParserError>> {
        None
    }
}
