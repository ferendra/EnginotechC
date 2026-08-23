// List across functions — VALUE semantics (like C pointers):
// append inside a callee builds a NEW buffer; rebind the result in the caller.
// Header layout [len][e0..en] makes len()/for-in/indexing work on ANY list.
fn total(xs: array) -> int {
    let mut s = 0;
    for v in xs {
        s += v;
    }
    return s;
}

fn count(xs: array) -> int {
    return len(xs);
}

fn push(xs: array, v: int) -> array {
    xs.append(v);
    return xs;          // return the grown list
}

fn main() {
    let nums = [10, 20, 30];

    assert(count(nums) == 3);
    assert(total(nums) == 60);

    let mut nums2 = push(nums, 40);
    assert(count(nums2) == 4);
    assert(nums2[3] == 40);
    assert(total(nums2) == 100);

    // original untouched (value semantics)
    assert(len(nums) == 3);

    nums2 = push(nums2, 50);
    assert(count(nums2) == 5);
    assert(total(nums2) == 150);

    print("test_list_fn OK");
}
