// flashlight_go.go — фонарик с режимом SOS на Go (консоль)

package main

import (
	"bufio"
	"fmt"
	"os"
	"strconv"
	"strings"
	"time"
)

type Flashlight struct {
	isOn      bool
	brightness int
	mode      string
	running   bool
	stopChan  chan bool
}

func NewFlashlight() *Flashlight {
	return &Flashlight{
		brightness: 80,
		mode:       "off",
		stopChan:   make(chan bool),
	}
}

func (f *Flashlight) turnOn() {
	f.stopCurrentMode()
	f.isOn = true
	f.mode = "on"
	f.running = true
	fmt.Println("Фонарик включён (постоянный свет)")
	f.updateIndicator()
}

func (f *Flashlight) turnOff() {
	f.stopCurrentMode()
	f.isOn = false
	f.mode = "off"
	f.running = false
	fmt.Println("Фонарик выключен")
}

func (f *Flashlight) setSOS() {
	f.stopCurrentMode()
	f.isOn = true
	f.mode = "sos"
	f.running = true
	fmt.Println("Режим SOS активирован")
	go f.sosLoop()
}

func (f *Flashlight) sosLoop() {
	f.stopChan = make(chan bool)
	for {
		select {
		case <-f.stopChan:
			return
		default:
			for i := 0; i < 3; i++ {
				f.flash(200)
				time.Sleep(200 * time.Millisecond)
			}
			for i := 0; i < 3; i++ {
				f.flash(600)
				time.Sleep(200 * time.Millisecond)
			}
			for i := 0; i < 3; i++ {
				f.flash(200)
				time.Sleep(200 * time.Millisecond)
			}
			time.Sleep(800 * time.Millisecond)
		}
	}
}

func (f *Flashlight) flash(ms int) {
	fmt.Print("\033[33m●\033[0m") // жёлтый
	time.Sleep(time.Duration(ms) * time.Millisecond)
	if !f.isOn || f.mode != "sos" {
		return
	}
	fmt.Print("\033[90m●\033[0m") // серый
}

func (f *Flashlight) setStrobe() {
	f.stopCurrentMode()
	f.isOn = true
	f.mode = "strobe"
	f.running = true
	fmt.Println("Режим стробоскопа активирован")
	go f.strobeLoop()
}

func (f *Flashlight) strobeLoop() {
	f.stopChan = make(chan bool)
	for {
		select {
		case <-f.stopChan:
			return
		default:
			fmt.Print("\033[97m●\033[0m") // белый
			time.Sleep(50 * time.Millisecond)
			fmt.Print("\033[90m●\033[0m") // серый
			time.Sleep(50 * time.Millisecond)
		}
	}
}

func (f *Flashlight) setFade() {
	f.stopCurrentMode()
	f.isOn = true
	f.mode = "fade"
	f.running = true
	fmt.Println("Режим затухания активирован")
	go f.fadeLoop()
}

func (f *Flashlight) fadeLoop() {
	f.stopChan = make(chan bool)
	for {
		select {
		case <-f.stopChan:
			return
		default:
			for i := 0; i <= 100; i += 5 {
				// анимация изменения яркости (вывод ASCII)
				fmt.Printf("\rЯркость: %d%% ", i)
				time.Sleep(50 * time.Millisecond)
			}
			for i := 100; i >= 0; i -= 5 {
				fmt.Printf("\rЯркость: %d%% ", i)
				time.Sleep(50 * time.Millisecond)
			}
		}
	}
}

func (f *Flashlight) stopCurrentMode() {
	f.running = false
	if f.mode != "off" && f.mode != "on" {
		close(f.stopChan)
		time.Sleep(100 * time.Millisecond)
	}
	f.mode = "off"
}

func (f *Flashlight) updateIndicator() {
	if f.isOn && f.mode == "on" {
		fmt.Printf("\rФонарик включён, яркость: %d%%", f.brightness)
	}
}

func (f *Flashlight) setBrightness(val int) {
	if val < 0 {
		val = 0
	}
	if val > 100 {
		val = 100
	}
	f.brightness = val
	if f.isOn && f.mode == "on" {
		fmt.Printf("\rЯркость установлена: %d%%", val)
	}
}

func main() {
	f := NewFlashlight()
	scanner := bufio.NewScanner(os.Stdin)
	fmt.Println("💡 Flashlight Pro — Go Edition")
	fmt.Println("Команды: on, off, sos, strobe, fade, bright <0-100>, info, exit")
	for {
		fmt.Print("> ")
		if !scanner.Scan() {
			break
		}
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}
		parts := strings.SplitN(line, " ", 2)
		cmd := parts[0]
		arg := ""
		if len(parts) > 1 {
			arg = parts[1]
		}
		switch cmd {
		case "on":
			f.turnOn()
		case "off":
			f.turnOff()
		case "sos":
			f.setSOS()
		case "strobe":
			f.setStrobe()
		case "fade":
			f.setFade()
		case "bright":
			if val, err := strconv.Atoi(arg); err == nil {
				f.setBrightness(val)
			} else {
				fmt.Println("Неверное число")
			}
		case "info":
			fmt.Printf("Состояние: %s, Яркость: %d%%, Режим: %s\n",
				map[bool]string{true: "вкл", false: "выкл"}[f.isOn],
				f.brightness,
				f.mode)
		case "exit":
			f.stopCurrentMode()
			fmt.Println("До свидания!")
			return
		default:
			fmt.Println("Неизвестная команда")
		}
	}
}
