
#[derive(Copy, Clone, Eq, PartialEq)]
pub enum TermType {
    TInt,
    TData,
    TList,
    TAtom,
    TFunc,
    TArray,
    TString,
    TObject,
    TFloat,
}

use self::TermType::*;

impl From<u64> for TermType {
    fn from(term_type: u64) -> TermType {
        match term_type {
            0 => TInt,
            1 => TData,
            2 => TList,
            3 => TAtom,
            4 => TFunc,
            5 => TArray,
            6 => TString,
            7 => TObject,
            _ => panic!("Invalid term header!")
        }
    }
}

impl TermType {
    #[inline]
    pub fn value(&self) -> u64 {
        match *self {
           TInt    => 0,
           TData   => 1,
           TList   => 2,
           TAtom   => 3,
           TFunc   => 4,
           TArray  => 5,
           TString => 6,
           TObject => 7,
           TFloat  => 8,
        }
    }
}