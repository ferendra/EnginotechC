// Integration test: boolean logic — and, or, not
fn main() {
    assert(true && true == true);
    assert(true && false == false);
    assert(false || true == true);
    assert(false || false == false);
    assert(not true == false);
    assert(not false == true);
    let a = true;
    let b = false;
    assert((a && b) == false);
    assert((a || b) == true);
    print("test_bool_logic OK");
}
