// M3 Module System Test
// Verifies: import resolution across files, declaration splicing, cycle safety

import mathutil;          // resolves tests/modules/mathutil.ec

fn main() {
    let a = double_it(21);   // from mathutil.ec → 42
    let b = square(5);       // from mathutil.ec → 25

    print("double_it(21) = " + str(a));
    print("square(5) = " + str(b));

    if a == 42 && b == 25 {
        print("M3 PASS");
    } else {
        print("M3 FAIL");
    }
}