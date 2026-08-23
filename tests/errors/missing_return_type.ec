// Error case: missing return type on non-main function
fn do_thing() {
    return 42;
}

fn main() {
    let result = do_thing();
    print(result);
}
