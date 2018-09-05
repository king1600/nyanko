use std::collections::LinkedList as List;

pub type SourceLoc = (usize, usize, usize);
pub type Expr<'a> = (ExprType<'a>, SourceLoc);
pub type Token<'a> = (TokenType<'a>, SourceLoc);

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
    Equ, Neq, Lt, Lte, Gt, Gte, Or, And,
}

#[derive(Debug)]
pub enum Pattern<'a> {
    PId(&'a [u8]),
    PTuple(List<Pattern<'a>>),
    PBind(&'a [u8], Box<Pattern<'a>>),
    PCons(Box<(Pattern<'a>, Pattern<'a>)>),
}

#[derive(Debug, Clone)]
pub enum TokenType<'a> {
    Int(i64),
    Float(f64),
    Kw(Keyword),
    Op(Operator),
    Id(&'a [u8]),
    Str(&'a [u8]),
    Dot, Colon, Comma, Arrow,
    LParen, RParen, LBrace, RBrace, LCurly, RCurly,
}

#[derive(Debug)]
pub enum ExprType<'a> {
    EInt(i64),
    EFloat(f64),
    EId(&'a [u8]),
    EStr(&'a [u8]),
    EAtom(&'a [u8]),
    EList(List<Expr<'a>>),
    ETuple(List<Expr<'a>>),
    EBlock(List<Expr<'a>>),
    EMap(List<(Expr<'a>, Expr<'a>)>),
    EModule(&'a [u8], List<Expr<'a>>),
    ECase(Box<Expr<'a>>, List<Pattern<'a>>),
    EFunc(&'a [u8], List<Pattern<'a>>, Option<Box<Expr<'a>>>, Box<Expr<'a>>),
    EIf(Box<(Expr<'a>, Expr<'a>)>, List<(Expr<'a>, Expr<'a>)>, Option<Box<Expr<'a>>>),
}