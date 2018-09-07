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
        c.is_alphanumeric() || c == '_' || c == '$'
    }

    fn next_char(&mut self) -> Option<char> {
        self.column += 1;
        if let Some('\n') = self.peek_char() {
            self.start += self.column;
            self.column = 0;
            self.line += 1;
        }
        self.chars.next()
    }

    fn read_while<P>(&mut self, 
        mut output: Option<&mut String>,
        predicate: P) where P: Fn(char) -> bool
    {
        while let Some(value) = self.peek_char() {
            if predicate(value) {
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
    fn consume(&mut self, token_type: TokenType, pos: SourceLoc) -> Result<Option<Token>, ParserError> {
        self.next_char();
        Ok(Some((token_type, pos)))
    }

    #[inline]
    fn line_comment(&mut self) -> Result<Option<Token>, ParserError> {
        self.read_while(None, |c| c != '\n');
        self.next()
    }

    #[inline]
    fn block_comment(&mut self, pos: SourceLoc) -> Result<Option<Token>, ParserError> {
        self.next_char();
        let mut depth = 1;
        loop {
            match (self.next_char(), self.next_char()) {
                (Some('*'), Some(')')) => depth -= 1,
                (Some('('), Some('*')) => depth += 1,
                (None, _) | (_, None)  => return Err(("Unterminated block comment", pos)),
                _ => {},
            }
            if depth == 0 {
                return self.next()
            }
        }
    }

    #[inline]
    fn scan_id(&mut self, pos: SourceLoc, start: char) -> Result<Option<Token>, ParserError> {
        let mut id = String::new();
        id.push(start);
        self.read_while(Some(&mut id), Self::is_identifier);
        Ok(Some((KEYWORDS.get(id.as_str()).unwrap_or(&Id(id)).clone(), pos)))
    }

    #[inline]
    fn scan_str(&mut self, pos: SourceLoc) -> Result<Option<Token>, ParserError> {
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
            Ok(Some((Str(string), pos)))
        } else {
            Err(("Unterminated string literal", pos))
        }
    }

    #[inline]
    fn scan_number(&mut self, pos: SourceLoc, start: char) -> Result<Option<Token>, ParserError> {
        let mut number = String::new();
        if start != '\0' {
            number.push(start);
        }

        self.read_while(Some(&mut number), |c| c.is_digit(10));
        let mut next = self.peek_char().unwrap_or('\0').to_ascii_lowercase();

        if next == '.' {
            number.push(self.next_char().unwrap_or('.'));
            self.read_while(Some(&mut number), |c| c.is_digit(10));
            next = self.peek_char().unwrap_or('\0').to_ascii_lowercase();
        }

        if next == 'e' {
            number.push(self.next_char().unwrap_or('e'));
            match (self.next_char(), self.peek_char()) {
                (Some(c), _) if c.is_digit(10) => 
                    self.read_while(Some(&mut number), |c| c.is_digit(10)),
                (Some('+'), Some(c)) | (Some('-'), Some(c)) if c.is_digit(10) => {
                    number.push(self.next_char().unwrap_or('+'));
                    self.read_while(Some(&mut number), |c| c.is_digit(10));
                },
                _ => return Err(("Invalid number literal", pos))
            }
        }

        let is_float = number.contains(".");
        Ok(if is_float || number.contains("e") {
            number.as_str().parse::<f64>().ok().and_then(|number| Some((
                if is_float { Float(number) } else { Int(number as i64) }
            , pos)))
        } else {
            number.as_str().parse::<i64>().ok().and_then(|int| Some((Int(int), pos)))
        })
    }

    pub fn next(&mut self) -> Result<Option<Token>, ParserError> {
        if !self.skip_whitespace() {
            return Ok(None)
        }

        let pos = (self.column, self.line, self.start);
        match (self.next_char(), self.peek_char()) {
            (Some('.'), Some(c)) if c.is_digit(10) => self.scan_number(pos, '\0'),
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
            (Some('('), Some('*')) => self.block_comment(pos),
            (Some('#'), _) => self.line_comment(),
            (Some('"'), _) => self.scan_str(pos),
            (Some('~'), _) => Ok(Some((Op(Bnot), pos))),
            (Some('&'), _) => Ok(Some((Op(Band), pos))),
            (Some('^'), _) => Ok(Some((Op(Xor), pos))),
            (Some('|'), _) => Ok(Some((Op(Bor), pos))),
            (Some('!'), _) => Ok(Some((Op(Not), pos))),
            (Some('+'), _) => Ok(Some((Op(Add), pos))),
            (Some('-'), _) => Ok(Some((Op(Sub), pos))),
            (Some('*'), _) => Ok(Some((Op(Mul), pos))),
            (Some('/'), _) => Ok(Some((Op(Div), pos))),
            (Some('%'), _) => Ok(Some((Op(Mod), pos))),
            (Some('='), _) => Ok(Some((Op(Set), pos))),
            (Some('('), _) => Ok(Some((LParen, pos))),
            (Some(')'), _) => Ok(Some((RParen, pos))),
            (Some('['), _) => Ok(Some((LBrace, pos))),
            (Some(']'), _) => Ok(Some((RBrace, pos))),
            (Some('{'), _) => Ok(Some((LCurly, pos))),
            (Some('}'), _) => Ok(Some((RCurly, pos))),
            (Some(','), _) => Ok(Some((Comma, pos))),
            (Some(':'), _) => Ok(Some((Colon, pos))),
            (Some('.'), _) => Ok(Some((Dot, pos))),
            _ => Err(("Unexpected character", pos)),
        }
    }
}