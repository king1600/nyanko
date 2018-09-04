use super::ast::*;
use self::Keyword::*;
use self::Operator::*;
use self::TokenType::*;

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
        if self.skip_whitespace() { return None }
        let pos = (self.column, self.line, self.start);

        match (self.next_byte(), self.peek_byte()) {
            (Some(b'.'), Some(byte)) if byte.is_ascii_digit() => self.scan_float(0, byte, pos),
            (Some(byte), _) if byte.is_ascii_digit() => self.scan_integer(pos),
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

    #[inline]
    fn peek_byte(&self) -> Option<u8> {
        self.source.get(self.pos).and_then(|byte| Some(*byte))
    }

    fn is_identifier(byte: u8) -> bool {
        byte.is_ascii_alphanumeric() || byte == b'_' || byte == b'$'
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
            if !predicate(byte) { break }
            self.next_byte();
            consumed += 1
        }
        consumed
    }

    fn skip_whitespace(&mut self) -> bool {
        self.read_while(|byte| byte.is_ascii_whitespace()) > 1
        && self.pos < self.source.len()
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
            if depth == 0 { break }
        }
        self.next()
    }

    fn scan_id(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        let start = self.pos - 1;
        if self.read_while(|byte| Self::is_identifier(byte)) > 0 {
            Some((Id(&self.source[start..self.pos]), source_loc))
        } else {
            None
        }
    }

    fn scan_integer(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        self.pos -= 1;
        self.scan_number().and_then(|(number, _)| {
            self.pos += 1;
            match self.peek_byte().unwrap_or(0).to_ascii_lowercase(){
                b'.' => self.scan_float(number, self.next_byte().unwrap_or(0), source_loc),
                b'e' => Some((Int(number * self.scan_exponent()), source_loc)),
                _ => Some((Int(number), source_loc))
            }
        })
    }

    fn scan_float(&mut self, start: i64, _point: u8, source_loc: SourceLoc) -> Option<Token<'a>> {
        self.scan_number().and_then(|(number, length)| {
            let number = (number as f64).powi(length as i32);
            if self.peek_byte().unwrap_or(0).to_ascii_lowercase() == b'e' {
                Some((Float(number * self.scan_exponent() as f64), source_loc))
            } else {
                Some((Float(number), source_loc))
            }
        })
    }

    fn scan_string(&mut self, source_loc: SourceLoc) -> Option<Token<'a>> {
        let start = self.pos;
        while self.peek_byte().is_some() {
            self.read_while(|byte| byte != b'"');
            self.next_byte();
            if self.source.get(self.pos - 2).unwrap_or(&0) != &b'\\' { break } 
        }

        if let Some(b'"') = self.source.get(self.pos - 1) {
            Some((Str(&self.source[start..self.pos - 2]), source_loc))
        } else {
            None
        }
    }

    fn scan_number(&mut self) -> Option<(i64, usize)> {
        use std::str::from_utf8;
        let length = self.read_while(|byte| byte.is_ascii_digit());

        (if length == 0 { None } else { Some(length) })
            .and_then(|length| from_utf8(&self.source[self.pos..length]).ok())
            .and_then(|string| string.parse::<i64>().ok())
            .and_then(|number| {
                self.pos += length;
                self.column += length;
                Some((number, length))
            })
    }

    fn scan_exponent(&mut self) -> i64 {
        self.next_byte();
        let modifier = match self.peek_byte() {
            Some(b'-') => { self.next_byte(); -1 },
            Some(b'+') => { self.next_byte(); 1 },
            _ => 1,
        };
        self.scan_number()
            .and_then(|(number, _)| Some(10i64.pow(number as u32) * modifier))
            .unwrap_or(0)
    }
}