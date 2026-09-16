use std::collections::hash_map::{HashMap};
use std::io::{Write};
use crate::stats::{Stats};
use std::fs::File;

pub fn print_results_to(cities_map: HashMap<String, Stats>, f: &mut File){
    // sort the keys
    let mut sorted_keys: Vec<&String> = cities_map.keys().collect();
    sorted_keys.sort();

    write!(f, "{{").unwrap();
    for city in sorted_keys{
        let stats = &cities_map[city].get();
        write!(f,"{city}:{0:.1}/{1:.1}/{2:.1}, ",
            stats.0,
            stats.1,
            stats.2
        ).unwrap();

    }
    write!(f, "}}").unwrap();
}

pub fn parse_temperature(s: &str)->i32{
    let bytes = s.as_bytes();
    let mut n = 0;
    let mut neg = false;
    for &b in bytes{
        match b {
            b'-' => neg = true,
            b'.' => {}
            b'0'..=b'9' => n = n*10 - (b-b'0') as i32,
            _ => break,
        }
    }
    if neg { n } else { -n }
}