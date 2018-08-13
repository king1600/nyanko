use std::fmt;

#[repr(u8)]
#[derive(Copy, Clone, Eq, PartialEq)]
pub enum TermType {
    TFloat,
    TInt,
    TData,
    TAtom,
    TFunc,
    TList,
    TString,
    TObject,
}

use self::TermType::*;

impl TermType {
    #[inline]
    pub fn value(&self) -> u64 {
        match *self {
           TInt    => 1,
           TData   => 2,
           TAtom   => 3,
           TFunc   => 4,
           TList   => 5,
           TString => 6,
           TObject => 7,
           TFloat  => 8,
        }
    }
}

impl fmt::Display for TermType {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{}", match *self {
            TInt => "Int",
            TData => "Data",
            TAtom => "Atom",
            TFunc => "Func",
            TList => "List",
            TString => "String",
            TObject => "Object",
            TFloat => "Float",
        })
    }
}

impl From<u64> for TermType {
    fn from(term_type: u64) -> TermType {
        match term_type {
            1 => TInt,
            2 => TData,
            3 => TAtom,
            4 => TFunc,
            5 => TList,
            6 => TString,
            7 => TObject,
            _ => panic!("Invalid term header!")
        }
    }
}