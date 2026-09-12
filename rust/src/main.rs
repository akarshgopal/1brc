use std::collections::hash_map::{HashMap};
use std::io::{BufRead, BufReader, Write};
use std::fs::File;

const MULT_FACTOR: f64 = 1.0;
const CAPACITY: usize = 10000;
#[derive(Debug, Clone)]
struct Stats{
    min:f64,
    max:f64,
    sum:f64,
    n:u64
}

impl Stats{
    fn new()-> Stats{
        Stats { min: f64::MAX, max: f64::MIN, sum: 0.0, n: 0 }
    }
}

// fn print_results(cities_map: HashMap<String, Stats>){
//     // sort the keys
//     let mut sorted_keys: Vec<&String> = cities_map.keys().collect();
//     sorted_keys.sort();

//     for city in sorted_keys{
//         let stats = &cities_map[city];
//         println!("{city}:{0:.1}/{1:.1}/{2:.1}",
//         (stats.min as f64)/(MULT_FACTOR as f64),
//         (stats.sum as f64)/(stats.n as f64)/(MULT_FACTOR as f64), 
//         (stats.max as f64)/(MULT_FACTOR as f64));
//     }
// }

fn print_results_to(cities_map: HashMap<String, Stats>, f: &mut File){
    // sort the keys
    let mut sorted_keys: Vec<&String> = cities_map.keys().collect();
    sorted_keys.sort();

    for city in sorted_keys{
        let stats = &cities_map[city];
        writeln!(f,"{city}:{0:.1}/{1:.1}/{2:.1}",
        (stats.min as f64)/(MULT_FACTOR as f64),
        (stats.sum as f64)/(stats.n as f64)/(MULT_FACTOR as f64), 
        (stats.max as f64)/(MULT_FACTOR as f64)).unwrap();
    }
}
fn main(){
    let mut cities_map: HashMap<String, Stats> = HashMap::with_capacity(CAPACITY);
    let path: &str = "../data/measurements_1B.txt";
    let f: File = File::open(path).unwrap();
    let reader = BufReader::new(f);
    let mut line = String::new();
    // allocate max capacity once
    line.reserve_exact(150);

    for line in reader.lines(){
        let line = line.unwrap();
        let (city, temperature) = line.split_once(';').unwrap();
        let temperature:f64 = temperature.parse().unwrap()  ;
        // update cities_map stats
        let stats = cities_map.entry(city.to_string()).or_insert(
            Stats::new()
        );

        stats.max = stats.max.max(temperature);
        stats.min = stats.min.min(temperature);
        stats.sum += temperature; 
        stats.n+= 1;

    }

    let mut f = File::create("../output/rs_sol.txt").unwrap();
    print_results_to(cities_map, &mut f);
}
