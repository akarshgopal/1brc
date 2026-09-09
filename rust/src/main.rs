use std::io::{BufRead, BufReader};
use std::fs::File;

fn main() -> std::io::Result<()> {
    let path = "../data/weather_stations.csv";
    let f = File::open(path)?;
    let mut reader = BufReader::new(f);
    let mut line = String::new();
    while reader.read_line(&mut line).unwrap()!=0{
        print!("{line}");
        line.clear();
    }
    Ok(())
}
