// M2 Struct Methods Test
// Methods declared directly inside struct body + dot notation calls

struct Point {
    x: int;
    y: int;

    fn distance(self) -> int {
        return self.x * self.x + self.y * self.y;
    }

    fn sum(self) -> int {
        return self.x + self.y;
    }
}

// Classic impl-block style still works too
impl Point {
    fn zero(self) -> int {
        return 0;
    }
}

fn main() {
    let p = Point { x: 3, y: 4 };

    // Method call via dot notation
    let d = p.distance();   // 3*3 + 4*4 = 25
    let s = p.sum();        // 3 + 4 = 7
    let z = p.zero();       // 0

    print("distance = " + str(d));
    print("sum = " + str(s));
    print("zero = " + str(z));

    if d == 25 {
        print("M2 PASS");
    } else {
        print("M2 FAIL");
    }
}