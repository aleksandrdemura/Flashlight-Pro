# flashlight_python.py — фонарик с режимом SOS на Python (Tkinter)

import tkinter as tk
from tkinter import ttk, messagebox
import json
import os
import threading
import time

class Flashlight:
    def __init__(self, root):
        self.root = root
        self.root.title("💡 Flashlight Pro — Python")
        self.root.geometry("500x400")
        self.is_on = False
        self.brightness = 80  # 0-100
        self.mode = "off"     # off, on, sos, strobe, fade
        self.running = False
        self.stop_event = threading.Event()
        self.config_file = "flashlight_config.json"
        self.load_config()
        self.create_widgets()
        self.update_status()
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def create_widgets(self):
        # Заголовок
        title = tk.Label(self.root, text="💡 Flashlight Pro", font=("Arial", 18))
        title.pack(pady=10)

        # Индикатор состояния
        self.status_label = tk.Label(self.root, text="Выключен", font=("Arial", 14), fg="gray")
        self.status_label.pack(pady=5)

        # Кнопки режимов
        btn_frame = tk.Frame(self.root)
        btn_frame.pack(pady=10)
        tk.Button(btn_frame, text="Вкл", command=self.turn_on, width=10, bg="green", fg="white").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Выкл", command=self.turn_off, width=10, bg="red", fg="white").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="SOS", command=self.set_sos, width=10, bg="orange").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Строб", command=self.set_strobe, width=10, bg="purple", fg="white").pack(side=tk.LEFT, padx=5)
        tk.Button(btn_frame, text="Затухание", command=self.set_fade, width=10, bg="blue", fg="white").pack(side=tk.LEFT, padx=5)

        # Яркость
        bright_frame = tk.Frame(self.root)
        bright_frame.pack(pady=10)
        tk.Label(bright_frame, text="Яркость:").pack(side=tk.LEFT)
        self.bright_slider = tk.Scale(bright_frame, from_=0, to=100, orient=tk.HORIZONTAL, length=200,
                                      command=self.set_brightness)
        self.bright_slider.set(self.brightness)
        self.bright_slider.pack(side=tk.LEFT, padx=10)
        self.bright_label = tk.Label(bright_frame, text=f"{self.brightness}%")
        self.bright_label.pack(side=tk.LEFT)

        # Индикатор (просто цветная метка)
        self.indicator = tk.Label(self.root, text="●", font=("Arial", 48), fg="gray")
        self.indicator.pack(pady=10)

        # Статус
        self.status = tk.Label(self.root, text="Готов", anchor=tk.W)
        self.status.pack(fill=tk.X, padx=10)

    def set_brightness(self, val):
        self.brightness = int(val)
        self.bright_label.config(text=f"{self.brightness}%")
        if self.is_on and self.mode == "on":
            # Имитация изменения яркости (обновление индикатора)
            self.update_indicator()
        self.save_config()

    def turn_on(self):
        self.stop_current_mode()
        self.is_on = True
        self.mode = "on"
        self.running = True
        self.update_indicator()
        self.status_label.config(text="Включен (постоянный)", fg="green")
        self.status.config(text="Режим: постоянный свет")
        self.save_config()

    def turn_off(self):
        self.stop_current_mode()
        self.is_on = False
        self.mode = "off"
        self.running = False
        self.indicator.config(fg="gray")
        self.status_label.config(text="Выключен", fg="gray")
        self.status.config(text="Выключен")
        self.save_config()

    def set_sos(self):
        self.stop_current_mode()
        self.is_on = True
        self.mode = "sos"
        self.running = True
        self.status_label.config(text="Режим SOS", fg="red")
        self.status.config(text="Сигнал бедствия ... --- ...")
        self.start_sos()
        self.save_config()

    def start_sos(self):
        self.stop_event.clear()
        threading.Thread(target=self._sos_loop, daemon=True).start()

    def _sos_loop(self):
        # SOS: три коротких, три длинных, три коротких, пауза
        while self.running and not self.stop_event.is_set():
            for _ in range(3):
                if self.stop_event.is_set(): break
                self.flash(0.2)
                time.sleep(0.2)
            for _ in range(3):
                if self.stop_event.is_set(): break
                self.flash(0.6)
                time.sleep(0.2)
            for _ in range(3):
                if self.stop_event.is_set(): break
                self.flash(0.2)
                time.sleep(0.2)
            time.sleep(0.8)  # пауза между циклами

    def flash(self, duration):
        self.indicator.config(fg="yellow")
        self.update_indicator()
        time.sleep(duration)
        if not self.stop_event.is_set():
            self.indicator.config(fg="gray")
            self.update_indicator()

    def set_strobe(self):
        self.stop_current_mode()
        self.is_on = True
        self.mode = "strobe"
        self.running = True
        self.status_label.config(text="Стробоскоп", fg="purple")
        self.status.config(text="Быстрое мигание")
        self.start_strobe()
        self.save_config()

    def start_strobe(self):
        self.stop_event.clear()
        threading.Thread(target=self._strobe_loop, daemon=True).start()

    def _strobe_loop(self):
        while self.running and not self.stop_event.is_set():
            self.indicator.config(fg="white")
            self.update_indicator()
            time.sleep(0.05)
            if self.stop_event.is_set(): break
            self.indicator.config(fg="gray")
            self.update_indicator()
            time.sleep(0.05)

    def set_fade(self):
        self.stop_current_mode()
        self.is_on = True
        self.mode = "fade"
        self.running = True
        self.status_label.config(text="Затухание", fg="blue")
        self.status.config(text="Плавное изменение яркости")
        self.start_fade()
        self.save_config()

    def start_fade(self):
        self.stop_event.clear()
        threading.Thread(target=self._fade_loop, daemon=True).start()

    def _fade_loop(self):
        while self.running and not self.stop_event.is_set():
            for i in range(0, 101, 5):
                if self.stop_event.is_set(): break
                self.brightness = i
                self.bright_slider.set(i)
                self.bright_label.config(text=f"{i}%")
                self.update_indicator()
                time.sleep(0.05)
            for i in range(100, -1, -5):
                if self.stop_event.is_set(): break
                self.brightness = i
                self.bright_slider.set(i)
                self.bright_label.config(text=f"{i}%")
                self.update_indicator()
                time.sleep(0.05)

    def update_indicator(self):
        # Имитация яркости: чем выше яркость, тем светлее индикатор
        if self.is_on and self.mode == "on":
            intensity = int(255 * self.brightness / 100)
            color = f"#{intensity:02x}{intensity:02x}{intensity:02x}"
            self.indicator.config(fg=color)
        elif self.mode in ("sos", "strobe"):
            # Цвет уже устанавливается в циклах
            pass
        elif self.mode == "fade":
            # Цвет обновляется в цикле
            pass

    def stop_current_mode(self):
        self.running = False
        self.stop_event.set()
        # Даем время потоку завершиться
        time.sleep(0.1)

    def update_status(self):
        # Обновление интерфейса
        if self.mode == "on":
            self.status_label.config(text="Включен", fg="green")
        elif self.mode == "sos":
            self.status_label.config(text="SOS", fg="red")
        elif self.mode == "strobe":
            self.status_label.config(text="Строб", fg="purple")
        elif self.mode == "fade":
            self.status_label.config(text="Затухание", fg="blue")
        else:
            self.status_label.config(text="Выключен", fg="gray")

    def load_config(self):
        if os.path.exists(self.config_file):
            with open(self.config_file, 'r') as f:
                data = json.load(f)
                self.brightness = data.get('brightness', 80)
                mode = data.get('mode', 'off')
                if mode == 'on':
                    self.turn_on()
                elif mode == 'sos':
                    self.set_sos()
                elif mode == 'strobe':
                    self.set_strobe()
                elif mode == 'fade':
                    self.set_fade()
                else:
                    self.turn_off()
                self.bright_slider.set(self.brightness)

    def save_config(self):
        data = {'brightness': self.brightness, 'mode': self.mode}
        with open(self.config_file, 'w') as f:
            json.dump(data, f)

    def on_close(self):
        self.stop_current_mode()
        self.save_config()
        self.root.destroy()

if __name__ == "__main__":
    root = tk.Tk()
    app = Flashlight(root)
    root.mainloop()
