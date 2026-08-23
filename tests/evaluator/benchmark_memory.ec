// Benchmark: Memory allocation stress
fn allocate_many() -> vector {
    let mut result: vector = [];
    for i in 0..1000 {
        let arr: vector = [i, i+1, i+2];
        result.append(arr);
    }
    return result;
}

fn main() {
    let data = allocate_many();
    print("allocated " + str(len(data)) + " arrays");
    print("memory_benchmark OK");
}
