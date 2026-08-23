// Integration test: strings — concat, interpolation, str() conversion
fn main() {
    let a = "foo";
    let b = "bar";
    assert(a + b == "foobar");

    let n = 42;
    assert(str(n) == "42");

    let name = "ec";
    let msg = "hello ${name}!";
    print(msg);

    print("test_string OK");
}
