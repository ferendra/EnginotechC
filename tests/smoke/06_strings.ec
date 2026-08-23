// Smoke 06 — strings: concat, compare, case, search, chr/ord
fn main() {
    let s = "enginotech";
    print(s + ".io");
    print("abc" == "abc");
    print("abc" == "abd");
    print("abc" < "abd");
    print("b" > "a");
    print(upper(s));
    print(lower("TECH"));
    print(contains(s, "tech"));
    print(contains(s, "xyz"));
    print(chr(65));
    print(ord("A"));
    print(len("hello"));
}
