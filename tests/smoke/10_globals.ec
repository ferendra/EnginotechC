// Smoke 10 — global variables & constants shared across functions
let counter = 10;
const APP = "Demo";
mut total = 0;

fn bump() {
    counter += 1;
    total += counter;
}

fn main() {
    print(APP);
    print(counter);
    bump();
    bump();
    print(counter);
    print(total);
    let local = counter * 2;
    print(local);
}
