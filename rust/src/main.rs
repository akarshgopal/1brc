use std::collections::hash_map::HashMap;
use std::fs::File;
use std::io::{BufRead, BufReader, Read};

mod helpers;
use helpers::{parse_temperature, print_results_to};

mod stats;
use stats::Stats;

const CAPACITY: usize = 10000;
const BUF_CHUNK: usize = 1024*1024;
const LEFTOVER_BUF: usize = 1024;
const BUF_SIZE: usize = BUF_CHUNK+LEFTOVER_BUF;

fn main() {
    let mut cities_map: HashMap<String, Stats> = HashMap::with_capacity(CAPACITY);
    let path: &str = "../data/measurements_10M.txt";
    let f: File = File::open(path).unwrap();

    // allocate max capacity once
    let mut buf = vec![0u8;BUF_SIZE];
    let leftover: usize = 0;
    
    loop{
        let n = f.read(&mut buf).unwrap();
        let total = n+leftover;
        if n==0{
            println!("EOF");
            break;
        } else if n <= BUF_CHUNK{
            println!("Leftover");
            break;
        }

        let start:usize = 0;
        let i:usize = 0;
        while i<total-LEFTOVER_BUF{
            if buf[i]==b';'{
                
            }
        }
    }

    for byte in reader.bytes() {
        let byte = byte.unwrap();
        let (city, tstr) = line.split_once(';').unwrap();
        let temperature: i32 = parse_temperature(tstr);
        // update cities_map stats
        let stats = cities_map.entry(city.to_string()).or_insert(Stats::new());
        stats.update(temperature);
    }

    let mut f = File::create("../output/rs_sol.txt").unwrap();
    print_results_to(cities_map, &mut f);
}
