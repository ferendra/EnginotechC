// Integration test: arithmetic, comparison, and assert
fn main() {
    assert(1 + 2 == 3);
    assert(10 - 4 == 6);
    assert(3 * 3 == 9);
    assert(20 / 4 == 5);
    assert(7 % 3 == 1);

    let mut x = 5;
    x = x + 1;
    assert(x == 6);
    x += 2;
    assert(x == 8);

    let f = 2.5;
    assert(f * 2.0 == 5.0);

    print("test_arith OK");
}
