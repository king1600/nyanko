use super::ast::*;
use std::str::Chars;
use self::Keyword::*;
use self::Operator::*;
use self::TokenType::*;
use std::iter::Peekable;
use std::collections::HashMap;

lazy_static! {
    static ref KEYWORDS: HashMap<&'static str, TokenType> = {
        let mut keywords = HashMap::new();
        keywords.insert("case", Kw(Case));
        keywords.insert("end",  Kw(End));
        keywords.insert("fn",   Kw(Fun));
        keywords.insert("mod",  Kw(Module));
        keywords.insert("if",   Kw(If));
        keywords.insert("elif", Kw(Elif));
        keywords.insert("else", Kw(Else));
        keywords
    };
}

pub struct Lexer<'a> {
    line: usize,
    start: usize,
    column: usize,
    chars: Peekable<Chars<'a>>,
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token;

    fn next(&mut self) -> Option<Token> {
        if !self.skip_whitespace() {
            return None
        }

        let pos = (self.column, self.line, self.start);
        match (self.next_char(), self.peek_char()) {
            (Some('.'), Some(c)) if c.is_digit(10) => self.scan_number(pos, c),
            (Some(c), _) if c.is_digit(10) => self.scan_number(pos, c),
            (Some(c), _) if Self::is_identifier(c) => self.scan_id(pos, c),
            (Some('>'), Some('>')) => self.consume(Op(Shr), pos),
            (Some('<'), Some('<')) => self.consume(Op(Shl), pos),
            (Some('='), Some('=')) => self.consume(Op(Equ), pos),
            (Some('!'), Some('=')) => self.consume(Op(Neq), pos),
            (Some('>'), Some('=')) => self.consume(Op(Gte), pos),
            (Some('<'), Some('=')) => self.consume(Op(Lte), pos),
            (Some('&'), Some('&')) => self.consume(Op(And), pos),
            (Some('|'), Some('|')) => self.consume(Op(Or), pos),
            (Some('='), Some('>')) => self.consume(Arrow, pos),
            (Some('('), Some('*')) => self.block_comment(),
            (Some('#'), _) => self.line_comment(),
            (Some('"'), _) => self.scan_str(pos),
            (Some('~'), _) => Some((Op(Bnot), pos)),
            (Some('&'), _) => Some((Op(Band), pos)),
            (Some('^'), _) => Some((Op(Xor), pos)),
            (Some('|'), _) => Some((Op(Bor), pos)),
            (Some('!'), _) => Some((Op(Not), pos)),
            (Some('+'), _) => Some((Op(Add), pos)),
            (Some('-'), _) => Some((Op(Sub), pos)),
            (Some('*'), _) => Some((Op(Mul), pos)),
            (Some('/'), _) => Some((Op(Div), pos)),
            (Some('%'), _) => Some((Op(Mod), pos)),
            (Some('='), _) => Some((Op(Set), pos)),
            (Some('('), _) => Some((LParen, pos)),
            (Some(')'), _) => Some((RParen, pos)),
            (Some('['), _) => Some((LBrace, pos)),
            (Some(']'), _) => Some((RBrace, pos)),
            (Some('{'), _) => Some((LCurly, pos)),
            (Some('}'), _) => Some((RCurly, pos)),
            (Some(','), _) => Some((Comma, pos)),
            (Some(':'), _) => Some((Colon, pos)),
            (Some('.'), _) => Some((Dot, pos)),
            _ => None,
        }
    }
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a str) -> Lexer<'a> {
        Lexer {
            line: 1,
            start: 0,
            column: 0,
            chars: source.chars().peekable(),
        }
    }

    #[inline]
    fn peek_char(&mut self) -> Option<char> {
        self.chars.peek().and_then(|c| Some(*c))
    }

    fn is_identifier(c: char) -> bool {
        c.is_digit(10) || c == '_' || c == '$'
    }

    fn next_char(&mut self) -> Option<char> {
        self.column += 1;
        let next = self.peek_char();
        if let Some('\n') = next {
            self.start += self.column;
            self.column = 0;
            self.line += 1;
        }
        next
    }

    fn read_while<P>(&mut self, 
        mut output: Option<&mut String>,
        predicate: P) where P: Fn(char) -> bool
    {
        while let Some(value) = self.peek_char() {
            if !predicate(value) {
                self.next_char();
                if let Some(string) = &mut output {
                    string.push(value);
                }
            } else {
                break
            }
        }
    }

    #[inline]
    fn skip_whitespace(&mut self) -> bool {
        self.read_while(None, |c| c.is_whitespace());
        self.peek_char().is_some()
    }

    #[inline]
    fn consume(&mut self, token_type: TokenType, pos: SourceLoc) -> Option<Token> {
        self.next_char();
        Some((token_type, pos))
    }

    #[inline]
    fn line_comment(&mut self) -> Option<Token> {
        None
    }

    #[inline]
    fn block_comment(&mut self) -> Option<Token> {
        None
    }

    #[inline]
    fn scan_id(&mut self, pos: SourceLoc, start: char) -> Option<Token> {
        None
    }

    #[inline]
    fn scan_str(&mut self, pos: SourceLoc) -> Option<Token> {
        let mut string = String::new();

        while self.peek_char().is_some() {
            self.read_while(Some(&mut string), |c| c != '"');
            if string.ends_with("\\") {
                string.push(self.next_char().unwrap_or('"'));
            } else {
                break
            }
        }

        if let Some('"') = self.peek_char() {
            self.next_char();
            Some((Str(string), pos))
        } else {
            None
        }
    }

    #[inline]
    fn scan_number(&mut self, pos: SourceLoc, start: char) -> Option<Token> {
        None
    }
}