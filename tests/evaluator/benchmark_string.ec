// Benchmark: String concatenation stress
fn build_string(n: int) -> string {
    let mut result = "";
    for i in 0..n {
        result = result + "x";
    }
    return result;
}

fn main() {
    let s = build_string(1000);
    let l = len(s);
    print("string length = " + str(l));
    assert(l == 1000);
    print("string_benchmark OK");
}
