// Smoke 03 — while, for-range, break/continue
fn sumTo(n: int) -> int {
    let total = 0;
    let i = 1;
    while (i <= n) {
        total += i;
        i += 1;
    }
    return total;
}

fn rangeSum(n: int) -> int {
    let total = 0;
    for i in 1..n+1 {
        total += i;
    }
    return total;
}

fn countEvens(limit: int) -> int {
    let c = 0;
    for i in 0..limit {
        if (i % 2 == 1) {
            continue;
        }
        c += 1;
        if (c >= 3) {
            break;
        }
    }
    return c;
}

fn main() {
    print(str(sumTo(10)));
    print(str(rangeSum(10)));
    print(str(countEvens(10)));
}
