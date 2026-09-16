#[derive(Debug, Clone)]
pub struct Stats {
    min: i32,
    max: i32,
    sum: i64,
    n: u64,
}
const MULT_FACTOR: f64 = 10.0;

impl Stats {
    pub fn new() -> Stats {
        Stats {
            min: i32::MAX,
            max: i32::MIN,
            sum: 0,
            n: 0,
        }
    }

    pub fn update(&mut self, t: i32) {
        self.max = self.max.max(t);
        self.min = self.min.min(t);
        self.sum += t as i64;
        self.n += 1;
    }

    pub fn get(&self) -> (f64, f64, f64) {
        (
            (self.min as f64) / (MULT_FACTOR),
            (self.sum as f64) / (self.n as f64) / (MULT_FACTOR),
            (self.max as f64) / (MULT_FACTOR),
        )
    }
}
