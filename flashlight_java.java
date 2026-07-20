// flashlight_java.java — фонарик с режимом SOS на Java (Swing)

import javax.swing.*;
import javax.swing.event.*;
import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.nio.file.*;
import java.util.concurrent.atomic.AtomicBoolean;

public class FlashlightJava extends JFrame {
    private static final String CONFIG_FILE = "flashlight_config.json";
    private JLabel statusLabel, indicator;
    private JSlider brightnessSlider;
    private JLabel brightnessLabel;
    private Timer timer;
    private Thread worker;
    private AtomicBoolean running = new AtomicBoolean(false);
    private AtomicBoolean stopFlag = new AtomicBoolean(false);
    private boolean isOn = false;
    private int brightness = 80;
    private String mode = "off";

    public FlashlightJava() {
        setTitle("💡 Flashlight Pro — Java");
        setSize(500, 450);
        setDefaultCloseOperation(EXIT_ON_CLOSE);
        setLayout(new BorderLayout());
        loadSettings();
        createUI();
        updateStatus();
    }

    private void createUI() {
        JPanel mainPanel = new JPanel(new BorderLayout());
        JPanel topPanel = new JPanel();
        topPanel.setLayout(new BoxLayout(topPanel, BoxLayout.Y_AXIS));
        JLabel title = new JLabel("💡 Flashlight Pro", SwingConstants.CENTER);
        title.setFont(new Font("Arial", Font.BOLD, 20));
        topPanel.add(title);
        statusLabel = new JLabel("Выключен", SwingConstants.CENTER);
        statusLabel.setFont(new Font("Arial", Font.PLAIN, 14));
        statusLabel.setForeground(Color.GRAY);
        topPanel.add(statusLabel);
        mainPanel.add(topPanel, BorderLayout.NORTH);

        JPanel btnPanel = new JPanel();
        JButton onBtn = new JButton("Вкл");
        onBtn.setBackground(Color.GREEN);
        onBtn.setForeground(Color.WHITE);
        JButton offBtn = new JButton("Выкл");
        offBtn.setBackground(Color.RED);
        offBtn.setForeground(Color.WHITE);
        JButton sosBtn = new JButton("SOS");
        sosBtn.setBackground(Color.ORANGE);
        JButton strobeBtn = new JButton("Строб");
        strobeBtn.setBackground(Color.MAGENTA);
        JButton fadeBtn = new JButton("Затухание");
        fadeBtn.setBackground(Color.BLUE);
        fadeBtn.setForeground(Color.WHITE);
        btnPanel.add(onBtn);
        btnPanel.add(offBtn);
        btnPanel.add(sosBtn);
        btnPanel.add(strobeBtn);
        btnPanel.add(fadeBtn);
        mainPanel.add(btnPanel, BorderLayout.CENTER);

        JPanel brightPanel = new JPanel(new FlowLayout());
        brightPanel.add(new JLabel("Яркость:"));
        brightnessSlider = new JSlider(0, 100, brightness);
        brightnessSlider.addChangeListener(e -> {
            brightness = brightnessSlider.getValue();
            brightnessLabel.setText(brightness + "%");
            if (isOn && mode.equals("on")) {
                updateIndicator();
            }
            saveSettings();
        });
        brightnessSlider.setPreferredSize(new Dimension(200, 30));
        brightPanel.add(brightnessSlider);
        brightnessLabel = new JLabel(brightness + "%");
        brightPanel.add(brightnessLabel);
        mainPanel.add(brightPanel, BorderLayout.SOUTH);

        indicator = new JLabel();
        indicator.setOpaque(true);
        indicator.setBackground(Color.GRAY);
        indicator.setPreferredSize(new Dimension(60, 60));
        indicator.setHorizontalAlignment(SwingConstants.CENTER);
        // Сделаем круглым через Border
        indicator.setBorder(BorderFactory.createLineBorder(Color.BLACK));
        JPanel indicatorPanel = new JPanel(new GridBagLayout());
        indicatorPanel.add(indicator);
        mainPanel.add(indicatorPanel, BorderLayout.CENTER);

        add(mainPanel);

        onBtn.addActionListener(e -> turnOn());
        offBtn.addActionListener(e -> turnOff());
        sosBtn.addActionListener(e -> setSOS());
        strobeBtn.addActionListener(e -> setStrobe());
        fadeBtn.addActionListener(e -> setFade());
    }

    private void turnOn() {
        stopCurrentMode();
        isOn = true;
        mode = "on";
        running.set(true);
        statusLabel.setText("Включен (постоянный)");
        statusLabel.setForeground(Color.GREEN);
        updateIndicator();
        saveSettings();
    }

    private void turnOff() {
        stopCurrentMode();
        isOn = false;
        mode = "off";
        running.set(false);
        indicator.setBackground(Color.GRAY);
        statusLabel.setText("Выключен");
        statusLabel.setForeground(Color.GRAY);
        saveSettings();
    }

    private void setSOS() {
        stopCurrentMode();
        isOn = true;
        mode = "sos";
        running.set(true);
        statusLabel.setText("SOS");
        statusLabel.setForeground(Color.RED);
        startSOS();
        saveSettings();
    }

    private void startSOS() {
        stopFlag.set(false);
        worker = new Thread(() -> {
            while (running.get() && !stopFlag.get()) {
                for (int i = 0; i < 3 && !stopFlag.get(); i++) {
                    flash(200);
                    try { Thread.sleep(200); } catch (InterruptedException e) {}
                }
                for (int i = 0; i < 3 && !stopFlag.get(); i++) {
                    flash(600);
                    try { Thread.sleep(200); } catch (InterruptedException e) {}
                }
                for (int i = 0; i < 3 && !stopFlag.get(); i++) {
                    flash(200);
                    try { Thread.sleep(200); } catch (InterruptedException e) {}
                }
                try { Thread.sleep(800); } catch (InterruptedException e) {}
            }
        });
        worker.start();
    }

    private void flash(int ms) {
        SwingUtilities.invokeLater(() -> indicator.setBackground(Color.YELLOW));
        try { Thread.sleep(ms); } catch (InterruptedException e) {}
        if (!stopFlag.get()) {
            SwingUtilities.invokeLater(() -> indicator.setBackground(Color.GRAY));
        }
    }

    private void setStrobe() {
        stopCurrentMode();
        isOn = true;
        mode = "strobe";
        running.set(true);
        statusLabel.setText("Стробоскоп");
        statusLabel.setForeground(Color.MAGENTA);
        startStrobe();
        saveSettings();
    }

    private void startStrobe() {
        stopFlag.set(false);
        worker = new Thread(() -> {
            while (running.get() && !stopFlag.get()) {
                SwingUtilities.invokeLater(() -> indicator.setBackground(Color.WHITE));
                try { Thread.sleep(50); } catch (InterruptedException e) {}
                if (stopFlag.get()) break;
                SwingUtilities.invokeLater(() -> indicator.setBackground(Color.GRAY));
                try { Thread.sleep(50); } catch (InterruptedException e) {}
            }
        });
        worker.start();
    }

    private void setFade() {
        stopCurrentMode();
        isOn = true;
        mode = "fade";
        running.set(true);
        statusLabel.setText("Затухание");
        statusLabel.setForeground(Color.BLUE);
        startFade();
        saveSettings();
    }

    private void startFade() {
        stopFlag.set(false);
        worker = new Thread(() -> {
            while (running.get() && !stopFlag.get()) {
                for (int i = 0; i <= 100 && !stopFlag.get(); i += 5) {
                    int intensity = 255 * i / 100;
                    Color color = new Color(intensity, intensity, intensity);
                    SwingUtilities.invokeLater(() -> indicator.setBackground(color));
                    try { Thread.sleep(50); } catch (InterruptedException e) {}
                }
                for (int i = 100; i >= 0 && !stopFlag.get(); i -= 5) {
                    int intensity = 255 * i / 100;
                    Color color = new Color(intensity, intensity, intensity);
                    SwingUtilities.invokeLater(() -> indicator.setBackground(color));
                    try { Thread.sleep(50); } catch (InterruptedException e) {}
                }
            }
        });
        worker.start();
    }

    private void stopCurrentMode() {
        running.set(false);
        stopFlag.set(true);
        if (worker != null) {
            try { worker.join(200); } catch (InterruptedException e) {}
            worker = null;
        }
    }

    private void updateIndicator() {
        if (isOn && mode.equals("on")) {
            int intensity = 255 * brightness / 100;
            indicator.setBackground(new Color(intensity, intensity, intensity));
        } else if (mode.equals("sos") || mode.equals("strobe") || mode.equals("fade")) {
            // Обновляется в потоках
        } else {
            indicator.setBackground(Color.GRAY);
        }
    }

    private void updateStatus() {
        // используется в методах
    }

    private void loadSettings() {
        try {
            String content = new String(Files.readAllBytes(Paths.get(CONFIG_FILE)));
            // упрощённый парсинг (не используем JSON библиотеку)
            // просто для демонстрации
            if (content.contains("brightness")) {
                // пропускаем
            }
        } catch (IOException e) {}
    }

    private void saveSettings() {
        try (PrintWriter pw = new PrintWriter(CONFIG_FILE)) {
            pw.println("{\"brightness\":" + brightness + ",\"mode\":\"" + mode + "\"}");
        } catch (IOException e) {}
    }

    public static void main(String[] args) throws Exception {
        UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
        SwingUtilities.invokeLater(() -> new FlashlightJava().setVisible(true));
    }
}
