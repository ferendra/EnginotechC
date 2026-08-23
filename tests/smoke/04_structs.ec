// Smoke 04 — struct literal, field read/write, method with receiver
struct Point {
    x: int;
    y: int;
}

impl Point {
    fn manhattan(p: Point) -> int {
        return p.x + p.y;
    }
}

fn main() {
    let p = Point { x: 3, y: 4 };
    print("x = " + str(p.x));
    print("y = " + str(p.y));
    print("manhattan = " + str(p.manhattan()));
    p.x = 10;
    print("moved x = " + str(p.x));
    print("new manhattan = " + str(p.manhattan()));
}
