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

fn pad0(v: f32, d: usize) -> String {
    let mut t = String::new();
    t.push_str(&v.to_string());
    if t.len() >= d {
        return t;
    } else {
        let mut tmp = String::from(fill("0", d-t.len()));
        tmp.push_str(&t);
        return tmp;
    }
}

fn pad_(v: f32, d: usize) -> String {
    let mut t = String::new();
    t.push_str(&v.to_string());
    if t.len() >= d {
        return t;
    } else {
        let mut tmp = String::from(fill(" ", d-t.len()));
        tmp.push_str(&t);
        return tmp;
    }
}

fn rpad_(v: f32, d: usize) -> String {
    let mut t = String::new();
    t.push_str(&v.to_string());
    if t.len() >= d {
        return t;
    } else {
        let tmp = String::from(fill(" ", d-t.len()));
        t.push_str(&tmp);
        return t;
    }
}

fn pad0r1(v: f32, d: usize) -> String {
    let mut t = String::new();
    let tv = v.round() as i16;
    t.push_str(&tv.to_string());
    if t.len() >= d {
        return t;
    } else {
        let mut tmp = String::from(fill("0", d-t.len()));
        tmp.push_str(&t);
        return tmp;
    }
}

fn pad0r2(v: f32, d: usize) -> String {
    let mut t = String::new();
    let tv = v.round() as i16;
    t.push_str(&tv.to_string());
    if t.len() >= d {
        return t;
    } else {
        let mut tmp = String::from(fill("0", d-t.len()));
        tmp.push_str(&t);
        return tmp;
    }
}

fn pad0r(v: f32, d:usize) -> String {
    let mut p2_32 = 2_f32;
    p2_32 = p2_32.powi(32);
    if v > p2_32 || v < -p2_32 {
        return pad0r1(v, d);
    }
    let i = v.round();
    return pad0r2(i, d)
}

fn isgeneral(s: &str, i: i16) {

}

fn main() {
    let a: &str = "conde";
    println!("{}", _strrev(a));
    println!("{}", fill(a, 5))

}
