use std::collections::LinkedList as List;

pub type Expr = (ExprType, SourceLoc);
pub type Token = (TokenType, SourceLoc);
pub type SourceLoc = (usize, usize, usize);

#[derive(Debug, Clone)]
pub enum Keyword {
    Case, End,
    Fun, Module,
    If, Elif, Else,
}

#[derive(Debug, Clone)]
pub enum Operator {
    Add, Sub, Div, Mul, Mod, Set,
    Shr, Shl, Xor, Bor, Bnot, Band,
    Equ, Neq, Lt, Lte, Gt, Gte, Or, And, Not,
}

#[derive(Debug)]
pub enum Pattern {
    PId(String),
    PTuple(List<Pattern>),
    PBind(String, Box<Pattern>),
    PCons(Box<(Pattern, Pattern)>),
}

#[derive(Debug, Clone)]
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