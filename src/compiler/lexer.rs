use super::ast::*;
use self::Keyword::*;
use self::Operator::*;
use self::TokenType::*;
use std::collections::HashMap;

lazy_static! {
    static ref KEYWORDS: HashMap<&'static [u8], TokenType<'static>> = {
        let mut keywords = HashMap::new();
        keywords.insert("mod".as_bytes(), Kw(Module));
        keywords.insert("fn".as_bytes(),    Kw(Fun));
        keywords.insert("case".as_bytes(),   Kw(Case));
        keywords.insert("end".as_bytes(),    Kw(End));
        keywords.insert("if".as_bytes(),     Kw(If));
        keywords.insert("elif".as_bytes(),   Kw(Elif));
        keywords.insert("else".as_bytes(),   Kw(Else));
        keywords
    };
}

pub struct Lexer<'a> {
    pos: usize,
    line: usize,
    start: usize,
    column: usize,
    source: &'a [u8]
}

impl<'a> Iterator for Lexer<'a> {
    type Item = Token<'a>;

    fn next(&mut self) -> Option<Token<'a>> {
        if !self.skip_whitespace() {
            return None
        }

        let pos = (self.column, self.line, self.start);
        match (self.next_byte(), self.peek_byte()) {
            (Some(b'.'), Some(byte)) if byte.is_ascii_digit() => self.scan_number(pos),
            (Some(byte), _) if byte.is_ascii_digit() => self.scan_number(pos),
            (Some(byte), _) if Self::is_identifier(byte) => self.scan_id(pos),
            (Some(b'>'), Some(b'>')) => self.consume(Op(Shr), pos),
            (Some(b'<'), Some(b'<')) => self.consume(Op(Shl), pos),
            (Some(b'='), Some(b'=')) => self.consume(Op(Equ), pos),
            (Some(b'!'), Some(b'=')) => self.consume(Op(Neq), pos),
            (Some(b'>'), Some(b'=')) => self.consume(Op(Gte), pos),
            (Some(b'<'), Some(b'=')) => self.consume(Op(Lte), pos),
            (Some(b'&'), Some(b'&')) => self.consume(Op(And), pos),
            (Some(b'|'), Some(b'|')) => self.consume(Op(Or), pos),
            (Some(b'('), Some(b'*')) => self.block_comment(),
            (Some(b'-'), Some(b'>')) => Some((Arrow, pos)),
            (Some(b'&'), _) => Some((Op(Band), pos)),
            (Some(b'~'), _) => Some((Op(Bnot), pos)),
            (Some(b'='), _) => Some((Op(Set), pos)),
            (Some(b'+'), _) => Some((Op(Add), pos)),
            (Some(b'-'), _) => Some((Op(Sub), pos)),
            (Some(b'|'), _) => Some((Op(Bor), pos)),
            (Some(b'^'), _) => Some((Op(Xor), pos)),
            (Some(b'*'), _) => Some((Op(Mul), pos)),
            (Some(b'/'), _) => Some((Op(Div), pos)),
            (Some(b'%'), _) => Some((Op(Mod), pos)),
            (Some(b'('), _) => Some((LParen, pos)),
            (Some(b')'), _) => Some((RParen, pos)),
            (Some(b'['), _) => Some((LBrace, pos)),
            (Some(b']'), _) => Some((RBrace, pos)),
            (Some(b'{'), _) => Some((LCurly, pos)),
            (Some(b'}'), _) => Some((RCurly, pos)),
            (Some(b','), _) => Some((Comma, pos)),
            (Some(b':'), _) => Some((Colon, pos)),
            (Some(b'.'), _) => Some((Dot, pos)),
            (Some(b'#'), _) => self.line_comment(),
            (Some(b'"'), _) => self.scan_string(pos),
            _ => None,
        }
    }
}

impl<'a> Lexer<'a> {
    pub fn new(source: &'a [u8]) -> Lexer<'a> {
        Lexer {
            pos: 0,
            line: 1,
            start: 0,
            column: 0,
            source: source,
        }
    }

    fn is_identifier(byte: u8) -> bool {
        byte.is_ascii_alphanumeric() || byte == b'_' || byte == b'$'
    }

    #[inline]
    fn peek_byte(&self) -> Option<u8> {
        self.source.get(self.pos).and_then(|byte| Some(*byte))
    }

    #[inline]
    fn next_if(&mut self, compare: u8) -> Option<u8> {
        if self.peek_byte().unwrap_or(0).to_ascii_lowercase() == compare {
            self.next_byte()
        } else {
            None
        }
    }

    fn next_byte(&mut self) -> Option<u8> {
        let byte = self.peek_byte();
        self.pos += 1;
        self.column += 1;
        if let Some(b'\n') = byte {
            self.start += self.column;
            self.column = 0;
            self.line += 1;
        }
        byte
    }

    fn read_while<P>(&mut self, predicate: P) -> usize where P: Fn(u8) -> bool {
        let mut consumed = 0;
        while let Some(&byte) = self.source.get(self.pos) {
            if !predicate(byte) {
                break
            }
            self.next_byte();
            consumed += 1
        }
        consumed
    }

    fn skip_whitespace(&mut self) -> bool {
        self.read_while(|byte| byte.is_ascii_whitespace()) > 0 || self.pos < self.source.len()
    }

    #[inline]
    fn consume(&mut self, token_type: TokenType<'a>, source_loc: SourceLoc) -> Option<Token<'a>> {
        self.next_byte();
        Some((token_type, source_loc))
    }

    fn line_comment(&mut self) -> Option<Token<'a>> {
        self.read_while(|byte| byte != b'\n');
        self.next_byte();
        self.next()
    }

    fn block_comment(&mut self) -> Option<Token<'a>> {
        let mut depth = 1;
        while let Some(byte) = self.next_byte() {
            match (byte, self.peek_byte()) {
                (b'(', Some(b'*')) => depth += 1,
                (b'*', Some(b')')) => depth -= 1,
                _ => {},
            }
            if depth == 0 {
                break
            }
        }
        self.next()
    }

    fn scan_id(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        let start = self.pos - 1;
        self.read_while(|byte| Self::is_identifier(byte));
        let name = &self.source[start..self.pos];
        Some((KEYWORDS.get(name).unwrap_or(&Id(name)).clone(), source_loc))
    }

    fn scan_string(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        let start = self.pos;
        while self.peek_byte().is_some() {
            self.read_while(|byte| byte != b'"');
            self.next_byte();
            if self.source.get(self.pos - 2).unwrap_or(&0) != &b'\\' {
                break
            } 
        }

        if let Some(b'"') = self.source.get(self.pos - 1) {
            Some((Str(&self.source[start..self.pos - 1]), source_loc))
        } else {
            None
        }
    }

    fn scan_number(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        use std::str::from_utf8;
        let start = self.pos - 1;

        self.read_while(|byte| 
            byte.is_ascii_digit() ||
            "eE+-.".as_bytes().contains(&byte));

        from_utf8(&self.source[start..self.pos]).ok().and_then(|text| {
            if text.contains("e") || text.contains("E") || text.contains(".") {
                text.parse::<f64>().ok().and_then(|number| {
                    Some((if text.contains(".") {
                        Float(number)
                    } else {
                        Int(number as i64)
                    }, source_loc))
                })
            } else {
                text.parse::<i64>().ok().and_then(|number| Some((Int(number), source_loc)))
            }
        })
    }
}