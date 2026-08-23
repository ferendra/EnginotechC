// Self-test untuk lib/std.ec (hanya fn main — digabung dengan std.ec oleh scripts/test_lib.sh)
fn main() {
    // string
    assert(strReverse("abc") == "cba");
    assert(strStartsWith("hello", "he"));
    assert(not strStartsWith("hello", "ell"));
    assert(strEndsWith("hello", "lo"));
    assert(strContainsChar("engi", 110));        // 'n'
    assert(not strContainsChar("engi", 122));    // 'z'
    assert(strCountChar("banana", 97) == 3);     // 'a'
    assert(strPadLeft("7", 48, 3) == "007");

    // list
    let xs = [5, 2, 9, 1];
    assert(listSum(xs) == 17);
    assert(listMax(xs) == 9);
    assert(listMin(xs) == 1);
    assert(listContains(xs, 9));
    assert(not listContains(xs, 7));

    let r = listRange(0, 5);
    assert(len(r) == 5);
    assert(r[4] == 4);
    assert(listSum(r) == 10);

    print("lib/std OK");
}
