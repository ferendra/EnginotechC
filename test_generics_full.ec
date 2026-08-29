// Generics integration test - uses stdlib types

import std.vec;
import std.option;
import std.result;

fn main() {
    // Test Vec
    let mut v: Vec<int> = Vec<int>::new();
    v.push(1);
    v.push(2);
    v.push(3);
    print("Vec len: " + str(v.len()));
    
    let first = v.get(0);
    print("First: " + str(first.unwrap_or(-1)));
    
    let missing = v.get(10);
    print("Missing: " + str(missing.unwrap_or(-1)));
    
    // Test Option
    let some_val: Option<string> = Option.Some("hello");
    let none_val: Option<int> = Option.None;
    
    print("Some is_some: " + str(some_val.is_some()));
    print("None is_none: " + str(none_val.is_none()));
    
    let mapped = some_val.map(|s| s + " world");
    print("Mapped: " + mapped.unwrap_or("empty"));
    
    // Test Result
    let ok: Result<int, string> = Result.Ok(42);
    let err: Result<int, string> = Result.Err("something failed");
    
    print("Ok is_ok: " + str(ok.is_ok()));
    print("Err is_err: " + str(err.is_err()));
    
    let doubled = ok.map(|x| x * 2);
    print("Doubled: " + str(doubled.unwrap_or(0)));
    
    let recovered = err.or_else(|e| Result.Ok(0));
    print("Recovered: " + str(recovered.unwrap_or(-1)));
    
    print("All generics tests passed!");
}