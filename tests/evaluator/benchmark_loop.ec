// Benchmark: Tight loop sum
fn main() {
    let mut total = 0;
    for i in 0..10000000 {
        total = total + i;
    }
    print("loop_sum = " + str(total));
    assert(total == 49999995000000 % 2147483647);  // modulo untuk i32 overflow
    print("loop_benchmark OK");
}
