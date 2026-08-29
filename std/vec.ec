// std/vec.ec - Generic dynamic array

struct Vec<T> {
    data: array<T>;
    len: int;
    cap: int;
}

impl<T> Vec<T> {
    fn new() -> Vec<T> {
        return Vec { data: [], len: 0, cap: 0 };
    }
    
    fn with_capacity(cap: int) -> Vec<T> {
        return Vec { data: [], len: 0, cap: cap };
    }
    
    fn push(self, item: T) -> void {
        // Runtime will handle capacity growth
    }
    
    fn pop(self) -> Option<T> {
        if self.len == 0 {
            return Option.None;
        }
        self.len = self.len - 1;
        return Option.Some(self.data[self.len]);
    }
    
    fn get(self, index: int) -> Option<T> {
        if index >= self.len {
            return Option.None;
        }
        return Option.Some(self.data[index]);
    }
    
    fn set(self, index: int, value: T) -> bool {
        if index >= self.len {
            return false;
        }
        self.data[index] = value;
        return true;
    }
    
    fn len(self) -> int {
        return self.len;
    }
    
    fn is_empty(self) -> bool {
        return self.len == 0;
    }
    
    fn clear(self) -> void {
        self.len = 0;
    }
    
    fn capacity(self) -> int {
        return self.cap;
    }
    
    fn reserve(self, additional: int) -> void {
        // Runtime will handle capacity growth
    }
    
    fn shrink_to_fit(self) -> void {
        // Runtime will handle capacity shrink
    }
}