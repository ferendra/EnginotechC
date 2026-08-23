// Integration test: while loop with break and continue
fn main() {
    let mut i = 0;
    let mut sum = 0;
    while i < 10 {
        i = i + 1;
        if i == 3 {
            continue;
        }
        if i == 7 {
            break;
        }
        sum = sum + i;
    }
    // Sum = 1+2+4+5+6 = 18
    assert(sum == 18);
    print("test_while_break_cont OK");
}
