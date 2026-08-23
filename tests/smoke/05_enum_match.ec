// Smoke 05 — enums + match with wildcard
enum Color {
    Red,
    Green,
    Blue,
}

fn classify(n: int) -> int {
    let r = 0;
    match n {
        0 => { r = 100; }
        1 => { r = 200; }
        _ => { r = 300; }
    }
    return r;
}

fn main() {
    let c = Color.Green;
    if (c == Color.Red) {
        print("red");
    } else {
        print("not red");
    }
    print(str(classify(0)));
    print(str(classify(1)));
    print(str(classify(9)));
}
