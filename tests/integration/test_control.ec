// Integration test: control flow — if/else, while, for-in ranges
fn main() {
    let n = 7;
    if n > 5 {
        print("big");
    } else {
        assert(false);
    }

    let mut i = 0;
    let mut total = 0;
    while i < 4 {
        total = total + i;
        i = i + 1;
    }
    assert(total == 6);

    let mut count = 0;
    for j in 0..10 {
        count = count + 1;
    }
    assert(count == 10);

    print("test_control OK");
}
