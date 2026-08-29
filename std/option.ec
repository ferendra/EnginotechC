// std/option.ec - Optional type (like Rust's Option)

enum Option<T> {
    Some(T),
    None
}

impl<T> Option<T> {
    fn is_some(self) -> bool {
        match self {
            Option.Some(_) => true,
            Option.None => false,
        }
    }
    
    fn is_none(self) -> bool {
        match self {
            Option.Some(_) => false,
            Option.None => true,
        }
    }
    
    fn unwrap(self) -> T {
        match self {
            Option.Some(val) => val,
            Option.None => panic("unwrap on None"),
        }
    }
    
    fn unwrap_or(self, default: T) -> T {
        match self {
            Option.Some(val) => val,
            Option.None => default,
        }
    }
    
    fn expect(self, msg: string) -> T {
        match self {
            Option.Some(val) => val,
            Option.None => panic(msg),
        }
    }
    
    fn map<U>(self, f: fn(T) -> U) -> Option<U> {
        match self {
            Option.Some(val) => Option.Some(f(val)),
            Option.None => Option.None,
        }
    }
    
    fn and_then<U>(self, f: fn(T) -> Option<U>) -> Option<U> {
        match self {
            Option.Some(val) => f(val),
            Option.None => Option.None,
        }
    }
    
    fn or_else(self, f: fn() -> Option<T>) -> Option<T> {
        match self {
            Option.Some(_) => self,
            Option.None => f(),
        }
    }
    
    fn filter(self, predicate: fn(T) -> bool) -> Option<T> {
        match self {
            Option.Some(val) if predicate(val) => Option.Some(val),
            _ => Option.None,
        }
    }
}