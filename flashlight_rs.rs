// flashlight_rs.rs — фонарик с режимом SOS на Rust (консоль + termion)

use std::io::{self, Write, BufRead};
use std::thread;
use std::time::Duration;
use std::sync::Arc;
use std::sync::atomic::{AtomicBool, Ordering};
use termion::{color, style};

struct Flashlight {
    is_on: bool,
    brightness: u8,
    mode: String,
    running: Arc<AtomicBool>,
    stop_flag: Arc<AtomicBool>,
}

impl Flashlight {
    fn new() -> Self {
        Flashlight {
            is_on: false,
            brightness: 80,
            mode: "off".to_string(),
            running: Arc::new(AtomicBool::new(false)),
            stop_flag: Arc::new(AtomicBool::new(false)),
        }
    }

    fn turn_on(&mut self) {
        self.stop_current_mode();
        self.is_on = true;
        self.mode = "on".to_string();
        self.running.store(true, Ordering::SeqCst);
        println!("Фонарик включён (постоянный свет)");
        self.update_indicator();
    }

    fn turn_off(&mut self) {
        self.stop_current_mode();
        self.is_on = false;
        self.mode = "off".to_string();
        self.running.store(false, Ordering::SeqCst);
        println!("Фонарик выключен");
    }

    fn set_sos(&mut self) {
        self.stop_current_mode();
        self.is_on = true;
        self.mode = "sos".to_string();
        self.running.store(true, Ordering::SeqCst);
        self.stop_flag.store(false, Ordering::SeqCst);
        println!("Режим SOS активирован");
        let running = self.running.clone();
        let stop_flag = self.stop_flag.clone();
        thread::spawn(move || {
            while running.load(Ordering::SeqCst) && !stop_flag.load(Ordering::SeqCst) {
                for _ in 0..3 {
                    if stop_flag.load(Ordering::SeqCst) { break; }
                    flash(200);
                    thread::sleep(Duration::from_millis(200));
                }
                for _ in 0..3 {
                    if stop_flag.load(Ordering::SeqCst) { break; }
                    flash(600);
                    thread::sleep(Duration::from_millis(200));
                }
                for _ in 0..3 {
                    if stop_flag.load(Ordering::SeqCst) { break; }
                    flash(200);
                    thread::sleep(Duration::from_millis(200));
                }
                thread::sleep(Duration::from_millis(800));
            }
        });
    }

    fn set_strobe(&mut self) {
        self.stop_current_mode();
        self.is_on = true;
        self.mode = "strobe".to_string();
        self.running.store(true, Ordering::SeqCst);
        self.stop_flag.store(false, Ordering::SeqCst);
        println!("Режим стробоскопа активирован");
        let running = self.running.clone();
        let stop_flag = self.stop_flag.clone();
        thread::spawn(move || {
            while running.load(Ordering::SeqCst) && !stop_flag.load(Ordering::SeqCst) {
                print!("{}{}●{}", color::Fg(color::White), style::Bold, style::Reset);
                io::stdout().flush().unwrap();
                thread::sleep(Duration::from_millis(50));
                if stop_flag.load(Ordering::SeqCst) { break; }
                print!("{}{}●{}", color::Fg(color::Black), style::Bold, style::Reset);
                io::stdout().flush().unwrap();
                thread::sleep(Duration::from_millis(50));
            }
        });
    }

    fn set_fade(&mut self) {
        self.stop_current_mode();
        self.is_on = true;
        self.mode = "fade".to_string();
        self.running.store(true, Ordering::SeqCst);
        self.stop_flag.store(false, Ordering::SeqCst);
        println!("Режим затухания активирован");
        let running = self.running.clone();
        let stop_flag = self.stop_flag.clone();
        thread::spawn(move || {
            while running.load(Ordering::SeqCst) && !stop_flag.load(Ordering::SeqCst) {
                for i in (0..=100).step_by(5) {
                    if stop_flag.load(Ordering::SeqCst) { break; }
                    print!("\r{}Яркость: {}%{}", color::Fg(color::Green), i, style::Reset);
                    io::stdout().flush().unwrap();
                    thread::sleep(Duration::from_millis(50));
                }
                for i in (0..=100).rev().step_by(5) {
                    if stop_flag.load(Ordering::SeqCst) { break; }
                    print!("\r{}Яркость: {}%{}", color::Fg(color::Green), i, style::Reset);
                    io::stdout().flush().unwrap();
                    thread::sleep(Duration::from_millis(50));
                }
            }
            println!();
        });
    }

    fn stop_current_mode(&mut self) {
        self.running.store(false, Ordering::SeqCst);
        self.stop_flag.store(true, Ordering::SeqCst);
        thread::sleep(Duration::from_millis(100));
        self.mode = "off".to_string();
    }

    fn update_indicator(&self) {
        if self.is_on && self.mode == "on" {
            println!("Яркость: {}%", self.brightness);
        }
    }

    fn set_brightness(&mut self, val: u8) {
        self.brightness = val;
        if self.is_on && self.mode == "on" {
            println!("Яркость установлена: {}%", val);
        }
    }
}

fn flash(ms: u64) {
    print!("{}{}●{}", color::Fg(color::Yellow), style::Bold, style::Reset);
    io::stdout().flush().unwrap();
    thread::sleep(Duration::from_millis(ms));
    print!("{}{}●{}", color::Fg(color::Black), style::Bold, style::Reset);
    io::stdout().flush().unwrap();
}

fn main() {
    let mut f = Flashlight::new();
    let stdin = io::stdin();
    let mut reader = stdin.lock();
    println!("{}💡 Flashlight Pro — Rust Edition{}", color::Fg(color::Cyan), style::Reset);
    println!("Команды: on, off, sos, strobe, fade, bright <0-100>, info, exit");
    loop {
        print!("{}> {} ", color::Fg(color::Yellow), style::Reset);
        io::stdout().flush().unwrap();
        let mut line = String::new();
        if reader.read_line(&mut line).is_err() { break; }
        let line = line.trim();
        if line.is_empty() { continue; }
        let parts: Vec<&str> = line.splitn(2, ' ').collect();
        let cmd = parts[0];
        let arg = if parts.len() > 1 { parts[1] } else { "" };
        match cmd {
            "on" => f.turn_on(),
            "off" => f.turn_off(),
            "sos" => f.set_sos(),
            "strobe" => f.set_strobe(),
            "fade" => f.set_fade(),
            "bright" => {
                if let Ok(val) = arg.parse::<u8>() {
                    f.set_brightness(val);
                } else {
                    println!("Неверное число");
                }
            }
            "info" => {
                println!("Состояние: {}, Яркость: {}%, Режим: {}",
                    if f.is_on { "вкл" } else { "выкл" },
                    f.brightness,
                    f.mode);
            }
            "exit" => {
                f.stop_current_mode();
                println!("До свидания!");
                break;
            }
            _ => println!("Неизвестная команда"),
        }
    }
}
