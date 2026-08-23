// Integration test: array of primitives with len and index
fn main() {
    let nums: vector = [1, 2, 3, 4, 5];
    let n = len(nums);
    assert(n == 5);
    assert(nums[0] == 1);
    assert(nums[4] == 5);
    let mut sum = 0;
    for x in nums {
        sum = sum + x;
    }
    assert(sum == 15);
    print("test_array_primitives OK");
}
