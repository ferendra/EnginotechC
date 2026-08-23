// Error case: syntax garbage — should produce multiple errors
fn main() {
    let x = ;
    if foo {
        bar = ;
    }
    print(x);
}
