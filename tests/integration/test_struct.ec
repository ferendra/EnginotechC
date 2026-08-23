// Integration test: structs — literals, field read/write
struct Point {
    x: int,
    y: int,
}

fn sum(p: Point) -> int {
    return p.x + p.y;
}

fn main() {
    let p = Point { x: 3, y: 4 };
    assert(p.x == 3);
    assert(p.y == 4);
    assert(sum(p) == 7);

    print("test_struct OK");
}
