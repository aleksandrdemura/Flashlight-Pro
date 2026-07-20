// flashlight_js.js — фонарик с режимом SOS на JavaScript (Node.js + readline)

const readline = require('readline');
const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout,
    prompt: '> '
});

class Flashlight {
    constructor() {
        this.isOn = false;
        this.brightness = 80;
        this.mode = 'off';
        this.running = false;
        this.stopFlag = false;
        this.interval = null;
    }

    turnOn() {
        this.stopCurrentMode();
        this.isOn = true;
        this.mode = 'on';
        this.running = true;
        console.log('Фонарик включён (постоянный свет)');
        this.updateIndicator();
    }

    turnOff() {
        this.stopCurrentMode();
        this.isOn = false;
        this.mode = 'off';
        this.running = false;
        console.log('Фонарик выключен');
    }

    setSOS() {
        this.stopCurrentMode();
        this.isOn = true;
        this.mode = 'sos';
        this.running = true;
        this.stopFlag = false;
        console.log('Режим SOS активирован');
        this.startSOS();
    }

    startSOS() {
        const sosPattern = [
            { duration: 200, count: 3 },  // короткие
            { duration: 600, count: 3 },  // длинные
            { duration: 200, count: 3 },  // короткие
            { duration: 800, count: 0 }   // пауза
        ];
        let step = 0;
        let count = 0;
        this.interval = setInterval(() => {
            if (this.stopFlag || !this.running) {
                clearInterval(this.interval);
                return;
            }
            const pattern = sosPattern[step];
            if (pattern.count === 0) {
                // пауза
                step = (step + 1) % sosPattern.length;
                count = 0;
                return;
            }
            if (count < pattern.count) {
                this.flash(pattern.duration);
                count++;
            } else {
                step = (step + 1) % sosPattern.length;
                count = 0;
            }
        }, 200);
    }

    flash(duration) {
        process.stdout.write('\x1b[33m●\x1b[0m'); // жёлтый
        setTimeout(() => {
            if (!this.stopFlag) {
                process.stdout.write('\x1b[90m●\x1b[0m'); // серый
            }
        }, duration);
    }

    setStrobe() {
        this.stopCurrentMode();
        this.isOn = true;
        this.mode = 'strobe';
        this.running = true;
        this.stopFlag = false;
        console.log('Режим стробоскопа активирован');
        this.startStrobe();
    }

    startStrobe() {
        this.interval = setInterval(() => {
            if (this.stopFlag || !this.running) {
                clearInterval(this.interval);
                return;
            }
            process.stdout.write('\x1b[97m●\x1b[0m'); // белый
            setTimeout(() => {
                if (!this.stopFlag && this.running) {
                    process.stdout.write('\x1b[90m●\x1b[0m'); // серый
                }
            }, 50);
        }, 100);
    }

    setFade() {
        this.stopCurrentMode();
        this.isOn = true;
        this.mode = 'fade';
        this.running = true;
        this.stopFlag = false;
        console.log('Режим затухания активирован');
        this.startFade();
    }

    startFade() {
        let direction = 1;
        let value = 0;
        this.interval = setInterval(() => {
            if (this.stopFlag || !this.running) {
                clearInterval(this.interval);
                return;
            }
            // выводим прогресс-бар яркости
            const bar = '█'.repeat(Math.floor(value/5)) + '░'.repeat(20 - Math.floor(value/5));
            process.stdout.write(`\rЯркость: ${value}% [${bar}]`);
            value += direction * 5;
            if (value >= 100 || value <= 0) direction *= -1;
        }, 50);
    }

    stopCurrentMode() {
        this.running = false;
        this.stopFlag = true;
        if (this.interval) {
            clearInterval(this.interval);
            this.interval = null;
        }
        this.mode = 'off';
    }

    updateIndicator() {
        if (this.isOn && this.mode === 'on') {
            console.log(`Яркость: ${this.brightness}%`);
        }
    }

    setBrightness(val) {
        val = Math.max(0, Math.min(100, val));
        this.brightness = val;
        if (this.isOn && this.mode === 'on') {
            console.log(`Яркость установлена: ${val}%`);
        }
    }

    interactive() {
        console.log('💡 Flashlight Pro — JavaScript Edition');
        console.log('Команды: on, off, sos, strobe, fade, bright <0-100>, info, exit');
        rl.prompt();

        rl.on('line', (line) => {
            const parts = line.trim().split(' ');
            const cmd = parts[0];
            const arg = parts.slice(1).join(' ');
            switch (cmd) {
                case 'on': this.turnOn(); break;
                case 'off': this.turnOff(); break;
                case 'sos': this.setSOS(); break;
                case 'strobe': this.setStrobe(); break;
                case 'fade': this.setFade(); break;
                case 'bright':
                    const val = parseInt(arg);
                    if (!isNaN(val)) this.setBrightness(val);
                    else console.log('Неверное число');
                    break;
                case 'info':
                    console.log(`Состояние: ${this.isOn ? 'вкл' : 'выкл'}, Яркость: ${this.brightness}%, Режим: ${this.mode}`);
                    break;
                case 'exit':
                    this.stopCurrentMode();
                    console.log('До свидания!');
                    rl.close();
                    return;
                default:
                    console.log('Неизвестная команда');
            }
            rl.prompt();
        }).on('close', () => process.exit(0));
    }
}

const app = new Flashlight();
app.interactive();
