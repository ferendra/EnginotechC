// std/result.ec - Result type for error handling (like Rust's Result)

enum Result<T, E> {
    Ok(T),
    Err(E)
}

impl<T, E> Result<T, E> {
    fn is_ok(self) -> bool {
        match self {
            Result.Ok(_) => true,
            Result.Err(_) => false,
        }
    }
    
    fn is_err(self) -> bool {
        match self {
            Result.Ok(_) => false,
            Result.Err(_) => true,
        }
    }
    
    fn unwrap(self) -> T {
        match self {
            Result.Ok(val) => val,
            Result.Err(e) => panic("unwrap on Err: " + str(e)),
        }
    }
    
    fn unwrap_err(self) -> E {
        match self {
            Result.Ok(v) => panic("unwrap_err on Ok: " + str(v)),
            Result.Err(e) => e,
        }
    }
    
    fn expect(self, msg: string) -> T {
        match self {
            Result.Ok(val) => val,
            Result.Err(e) => panic(msg + ": " + str(e)),
        }
    }
    
    fn expect_err(self, msg: string) -> E {
        match self {
            Result.Ok(v) => panic(msg + ": " + str(v)),
            Result.Err(e) => e,
        }
    }
    
    fn unwrap_or(self, default: T) -> T {
        match self {
            Result.Ok(val) => val,
            Result.Err(_) => default,
        }
    }
    
    fn map<U>(self, f: fn(T) -> U) -> Result<U, E> {
        match self {
            Result.Ok(val) => Result.Ok(f(val)),
            Result.Err(e) => Result.Err(e),
        }
    }
    
    fn map_err<F>(self, f: fn(E) -> F) -> Result<T, F> {
        match self {
            Result.Ok(val) => Result.Ok(val),
            Result.Err(e) => Result.Err(f(e)),
        }
    }
    
    fn and_then<U>(self, f: fn(T) -> Result<U, E>) -> Result<U, E> {
        match self {
            Result.Ok(val) => f(val),
            Result.Err(e) => Result.Err(e),
        }
    }
    
    fn or_else(self, f: fn(E) -> Result<T, E>) -> Result<T, E> {
        match self {
            Result.Ok(_) => self,
            Result.Err(e) => f(e),
        }
    }
}