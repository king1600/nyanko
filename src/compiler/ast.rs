use std::fmt;
use std::collections::LinkedList as InnerList;

pub type List<T> = InnerList<T>;
pub type Expr = (ExprType, SourceLoc);
pub type Token = (TokenType, SourceLoc);
pub type SourceLoc = (usize, usize, usize);
pub type ParserError = (String, SourceLoc);

#[derive(Debug, Clone, PartialEq)]
pub enum Keyword {
    Case, End,
    Fun, Module,
    If, Elif, Else,
}

#[derive(Debug, Clone, PartialEq)]
pub enum Operator {
    Add, Sub, Div, Mul, Mod, Set,
    Shr, Shl, Xor, Bor, Bnot, Band,
    Equ, Neq, Lt, Lte, Gt, Gte, Or, And, Not,
}

#[derive(Debug, Clone, PartialEq)]
pub enum TokenType {
    Int(i64),
    Id(String),
    Str(String),
    Float(f64),
    Kw(Keyword),
    Op(Operator),
    Dot, Colon, Comma, Arrow,
    LParen, RParen, LBrace, RBrace, LCurly, RCurly,
}

#[derive(Debug)]
pub enum Pattern {
    PId(String),
    PTuple(List<Pattern>),
    PBind(String, Box<Pattern>),
    PCons(Box<(Pattern, Pattern)>),
}

#[derive(Debug)]
pub enum ExprType {
    EInt(i64),
    EFloat(f64),
    EId(String),
    EStr(String),
    EAtom(String),
    EList(List<Expr>),
    ETuple(List<Expr>),
    EBlock(List<Expr>),
    EMap(List<(Expr, Expr)>),
    EModule(String, List<Expr>),
    ECase(Box<Expr>, List<Pattern>),
    EFunc(String, List<Pattern>, Option<Box<Expr>>, Box<Expr>),
    EIf(Box<(Expr, Expr)>, List<(Expr, Expr)>, Option<Box<Expr>>),
}

impl fmt::Display for TokenType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        use self::TokenType::*;
        use self::Operator::*;
        use self::Keyword::*;

        write!(f, "{}", match self {
            &Dot => ".".to_string(),
            &Colon => ":".to_string(),
            &Comma => ",".to_string(),
            &Arrow => "->".to_string(),
            &LParen => "(".to_string(),
            &RParen => ")".to_string(),
            &LBrace => "[".to_string(),
            &RBrace => "]".to_string(),
            &LCurly => "{".to_string(),
            &RCurly => "}".to_string(),
            &Kw(Case)   => "case".to_string(),
            &Kw(End)    => "end".to_string(),
            &Kw(Fun)    => "fn".to_string(),
            &Kw(Module) => "mod".to_string(),
            &Kw(If)     => "if".to_string(),
            &Kw(Elif)   => "elif".to_string(),
            &Kw(Else)   => "else".to_string(),
            &Id(ref id) => id.clone(),
            &Int(int) => int.to_string(),
            &Float(float) => float.to_string(),
            &Str(ref string) => string.clone(),
            &Op(Add)  => "+".to_string(),
            &Op(Sub)  => "-".to_string(),
            &Op(Mul)  => "*".to_string(),
            &Op(Div)  => "/".to_string(),
            &Op(Mod)  => "%".to_string(),
            &Op(Set)  => "=".to_string(),
            &Op(Shr)  => ">>".to_string(),
            &Op(Shl)  => "<<".to_string(),
            &Op(Xor)  => "^".to_string(),
            &Op(Band) => "&".to_string(),
            &Op(Bor)  => "|".to_string(),
            &Op(Bnot) => "~".to_string(),
            &Op(Equ)  => "==".to_string(),
            &Op(Neq)  => "!=".to_string(),
            &Op(Lt)   => "<".to_string(),
            &Op(Lte)  => "<=".to_string(),
            &Op(Gt)   => ">".to_string(),
            &Op(Gte)  => ">=".to_string(),
            &Op(And)  => "&&".to_string(),
            &Op(Or)   => "||".to_string(),
            &Op(Not)  => "!".to_string(),
        })
    }
}