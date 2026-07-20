// flashlight_cs.cs — фонарик с режимом SOS на C# (WPF)

using System;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Threading;

namespace FlashlightWPF
{
    public partial class MainWindow : Window
    {
        private bool isOn = false;
        private int brightness = 80;
        private string mode = "off";
        private CancellationTokenSource cts;
        private Task workerTask;
        private bool running = false;
        private DispatcherTimer timer;

        private Label statusLabel, indicator;
        private Slider brightnessSlider;
        private Label brightnessLabel;

        public MainWindow()
        {
            InitializeComponent();
            LoadSettings();
            CreateUI();
            UpdateStatus();
        }

        private void CreateUI()
        {
            Title = "💡 Flashlight Pro — C#";
            Width = 500;
            Height = 450;
            var grid = new Grid();
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
            grid.RowDefinitions.Add(new RowDefinition { Height = new GridLength(1, GridUnitType.Star) });
            grid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });

            var title = new Label { Content = "💡 Flashlight Pro", FontSize = 20, FontWeight = FontWeights.Bold, HorizontalAlignment = HorizontalAlignment.Center };
            Grid.SetRow(title, 0);
            grid.Children.Add(title);

            statusLabel = new Label { Content = "Выключен", FontSize = 14, Foreground = Brushes.Gray, HorizontalAlignment = HorizontalAlignment.Center };
            Grid.SetRow(statusLabel, 1);
            grid.Children.Add(statusLabel);

            var btnPanel = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Center };
            var onBtn = new Button { Content = "Вкл", Background = Brushes.Green, Foreground = Brushes.White, Width = 60 };
            var offBtn = new Button { Content = "Выкл", Background = Brushes.Red, Foreground = Brushes.White, Width = 60 };
            var sosBtn = new Button { Content = "SOS", Background = Brushes.Orange, Width = 60 };
            var strobeBtn = new Button { Content = "Строб", Background = Brushes.Purple, Foreground = Brushes.White, Width = 60 };
            var fadeBtn = new Button { Content = "Затухание", Background = Brushes.Blue, Foreground = Brushes.White, Width = 80 };
            btnPanel.Children.Add(onBtn);
            btnPanel.Children.Add(offBtn);
            btnPanel.Children.Add(sosBtn);
            btnPanel.Children.Add(strobeBtn);
            btnPanel.Children.Add(fadeBtn);
            Grid.SetRow(btnPanel, 2);
            grid.Children.Add(btnPanel);

            var brightPanel = new StackPanel { Orientation = Orientation.Horizontal, HorizontalAlignment = HorizontalAlignment.Center, Margin = new Thickness(5) };
            brightPanel.Children.Add(new Label { Content = "Яркость:" });
            brightnessSlider = new Slider { Minimum = 0, Maximum = 100, Value = brightness, Width = 200 };
            brightnessSlider.ValueChanged += (s, e) => {
                brightness = (int)brightnessSlider.Value;
                brightnessLabel.Content = brightness + "%";
                if (isOn && mode == "on") UpdateIndicator();
                SaveSettings();
            };
            brightPanel.Children.Add(brightnessSlider);
            brightnessLabel = new Label { Content = brightness + "%" };
            brightPanel.Children.Add(brightnessLabel);
            Grid.SetRow(brightPanel, 3);
            grid.Children.Add(brightPanel);

            indicator = new Label { Content = "", Width = 60, Height = 60, Background = Brushes.Gray, HorizontalContentAlignment = HorizontalAlignment.Center, VerticalContentAlignment = VerticalAlignment.Center };
            indicator.Margin = new Thickness(0, 10, 0, 10);
            Grid.SetRow(indicator, 4);
            grid.Children.Add(indicator);

            Content = grid;

            onBtn.Click += (s, e) => TurnOn();
            offBtn.Click += (s, e) => TurnOff();
            sosBtn.Click += (s, e) => SetSOS();
            strobeBtn.Click += (s, e) => SetStrobe();
            fadeBtn.Click += (s, e) => SetFade();
        }

        private void TurnOn()
        {
            StopCurrentMode();
            isOn = true;
            mode = "on";
            running = true;
            statusLabel.Content = "Включен (постоянный)";
            statusLabel.Foreground = Brushes.Green;
            UpdateIndicator();
            SaveSettings();
        }

        private void TurnOff()
        {
            StopCurrentMode();
            isOn = false;
            mode = "off";
            running = false;
            indicator.Background = Brushes.Gray;
            statusLabel.Content = "Выключен";
            statusLabel.Foreground = Brushes.Gray;
            SaveSettings();
        }

        private void SetSOS()
        {
            StopCurrentMode();
            isOn = true;
            mode = "sos";
            running = true;
            statusLabel.Content = "SOS";
            statusLabel.Foreground = Brushes.Red;
            StartSOS();
            SaveSettings();
        }

        private async void StartSOS()
        {
            cts = new CancellationTokenSource();
            var token = cts.Token;
            await Task.Run(async () => {
                while (running && !token.IsCancellationRequested)
                {
                    for (int i = 0; i < 3 && !token.IsCancellationRequested; i++)
                    {
                        await Flash(200);
                        await Task.Delay(200, token);
                    }
                    for (int i = 0; i < 3 && !token.IsCancellationRequested; i++)
                    {
                        await Flash(600);
                        await Task.Delay(200, token);
                    }
                    for (int i = 0; i < 3 && !token.IsCancellationRequested; i++)
                    {
                        await Flash(200);
                        await Task.Delay(200, token);
                    }
                    await Task.Delay(800, token);
                }
            });
        }

        private async Task Flash(int ms)
        {
            await Dispatcher.InvokeAsync(() => indicator.Background = Brushes.Yellow);
            await Task.Delay(ms);
            if (!cts.IsCancellationRequested)
                await Dispatcher.InvokeAsync(() => indicator.Background = Brushes.Gray);
        }

        private void SetStrobe()
        {
            StopCurrentMode();
            isOn = true;
            mode = "strobe";
            running = true;
            statusLabel.Content = "Стробоскоп";
            statusLabel.Foreground = Brushes.Purple;
            StartStrobe();
            SaveSettings();
        }

        private async void StartStrobe()
        {
            cts = new CancellationTokenSource();
            var token = cts.Token;
            await Task.Run(async () => {
                while (running && !token.IsCancellationRequested)
                {
                    await Dispatcher.InvokeAsync(() => indicator.Background = Brushes.White);
                    await Task.Delay(50, token);
                    if (token.IsCancellationRequested) break;
                    await Dispatcher.InvokeAsync(() => indicator.Background = Brushes.Gray);
                    await Task.Delay(50, token);
                }
            });
        }

        private void SetFade()
        {
            StopCurrentMode();
            isOn = true;
            mode = "fade";
            running = true;
            statusLabel.Content = "Затухание";
            statusLabel.Foreground = Brushes.Blue;
            StartFade();
            SaveSettings();
        }

        private async void StartFade()
        {
            cts = new CancellationTokenSource();
            var token = cts.Token;
            await Task.Run(async () => {
                while (running && !token.IsCancellationRequested)
                {
                    for (int i = 0; i <= 100 && !token.IsCancellationRequested; i += 5)
                    {
                        int intensity = 255 * i / 100;
                        var color = Color.FromRgb((byte)intensity, (byte)intensity, (byte)intensity);
                        await Dispatcher.InvokeAsync(() => indicator.Background = new SolidColorBrush(color));
                        await Task.Delay(50, token);
                    }
                    for (int i = 100; i >= 0 && !token.IsCancellationRequested; i -= 5)
                    {
                        int intensity = 255 * i / 100;
                        var color = Color.FromRgb((byte)intensity, (byte)intensity, (byte)intensity);
                        await Dispatcher.InvokeAsync(() => indicator.Background = new SolidColorBrush(color));
                        await Task.Delay(50, token);
                    }
                }
            });
        }

        private void StopCurrentMode()
        {
            running = false;
            if (cts != null)
            {
                cts.Cancel();
                cts.Dispose();
                cts = null;
            }
            // Ждём завершения задачи
            if (workerTask != null && !workerTask.IsCompleted)
                workerTask.Wait(100);
        }

        private void UpdateIndicator()
        {
            if (isOn && mode == "on")
            {
                int intensity = 255 * brightness / 100;
                indicator.Background = new SolidColorBrush(Color.FromRgb((byte)intensity, (byte)intensity, (byte)intensity));
            }
            else if (mode != "off")
            {
                // обновляется в потоках
            }
            else
            {
                indicator.Background = Brushes.Gray;
            }
        }

        private void LoadSettings()
        {
            // упрощённо
        }

        private void SaveSettings()
        {
            // упрощённо
        }

        [STAThread]
        static void Main()
        {
            var app = new Application();
            app.Run(new MainWindow());
        }
    }
}
