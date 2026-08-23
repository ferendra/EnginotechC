// Integration test: enums and equality
enum Color {
    Red,
    Green,
    Blue,
}

fn main() {
    let c = Color.Green;
    assert(c == Color.Green);
    assert(c != Color.Red);

    print("test_enum OK");
}
