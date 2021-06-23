fn _strrev(x: &str) -> String {
    return x.chars().rev().collect::<String>();
}

fn fill(c: &str, l: usize) -> String {
    let mut o = String::new();
    while o.len() < l {
        o.push_str(c);
    }
    return o;
}

fn pad0(v: &str, d: usize) -> String {
    let mut t = String::new();
    t.push_str(v);
    if t.len() >= d {
        return t;
    } else {
        let mut tmp = String::from(fill("0", d-t.len()));

        tmp.push_str(t);
    }
    return if t.len() >= d {t} else {fill("0", d-t.len())}

}

fn main() {
    let a: &str = "conde";
    println!("{}", _strrev(a));
    println!("{}", fill(a, 5))

}
