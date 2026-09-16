use std::collections::hash_map::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader};

mod helpers;
use helpers::{parse_temperature, print_results_to};

mod stats;
use stats::Stats;

const CAPACITY: usize = 10000;

fn main() {
    let mut cities_map: HashMap<String, Stats> = HashMap::with_capacity(CAPACITY);
    let path: &str = "../data/measurements_10M.txt";
    let f: File = File::open(path).unwrap();
    let reader = BufReader::new(f);
    let mut line = String::new();
    // allocate max capacity once
    line.reserve_exact(150);

    for line in reader.lines() {
        let line = line.unwrap();
        let (city, tstr) = line.split_once(';').unwrap();
        let temperature: i32 = parse_temperature(tstr);
        // update cities_map stats
        let stats = cities_map.entry(city.to_string()).or_insert(Stats::new());
        stats.update(temperature);
    }

    let mut f = File::create("../output/rs_sol.txt").unwrap();
    print_results_to(cities_map, &mut f);
}
