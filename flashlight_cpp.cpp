// flashlight_cpp.cpp — фонарик с режимом SOS на C++ (Qt)

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QSlider>
#include <QTimer>
#include <QSound>
#include <QSettings>
#include <QMessageBox>
#include <QThread>
#include <QColor>

class Flashlight : public QMainWindow {
    Q_OBJECT
public:
    Flashlight(QWidget *parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("💡 Flashlight Pro — C++");
        resize(500, 450);
        createUI();
        loadSettings();
        updateStatus();
    }

private slots:
    void turnOn() {
        stopCurrentMode();
        isOn = true;
        mode = "on";
        running = true;
        statusLabel->setText("Включен (постоянный)");
        statusLabel->setStyleSheet("color: green;");
        updateIndicator();
        saveSettings();
    }

    void turnOff() {
        stopCurrentMode();
        isOn = false;
        mode = "off";
        running = false;
        indicator->setStyleSheet("background-color: gray; border-radius: 30px;");
        statusLabel->setText("Выключен");
        statusLabel->setStyleSheet("color: gray;");
        saveSettings();
    }

    void setSOS() {
        stopCurrentMode();
        isOn = true;
        mode = "sos";
        running = true;
        statusLabel->setText("SOS");
        statusLabel->setStyleSheet("color: red;");
        startSOS();
        saveSettings();
    }

    void setStrobe() {
        stopCurrentMode();
        isOn = true;
        mode = "strobe";
        running = true;
        statusLabel->setText("Стробоскоп");
        statusLabel->setStyleSheet("color: purple;");
        startStrobe();
        saveSettings();
    }

    void setFade() {
        stopCurrentMode();
        isOn = true;
        mode = "fade";
        running = true;
        statusLabel->setText("Затухание");
        statusLabel->setStyleSheet("color: blue;");
        startFade();
        saveSettings();
    }

    void setBrightness(int val) {
        brightness = val;
        brightnessLabel->setText(QString::number(val) + "%");
        if (isOn && mode == "on") {
            updateIndicator();
        }
        saveSettings();
    }

private:
    QLabel *statusLabel;
    QLabel *indicator;
    QSlider *brightnessSlider;
    QLabel *brightnessLabel;
    QTimer *timer;
    bool isOn = false;
    int brightness = 80;
    QString mode = "off";
    bool running = false;
    QThread *workerThread = nullptr;
    bool stopFlag = false;

    void createUI() {
        QWidget *central = new QWidget(this);
        setCentralWidget(central);
        QVBoxLayout *mainLayout = new QVBoxLayout(central);

        QLabel *title = new QLabel("💡 Flashlight Pro");
        title->setAlignment(Qt::AlignCenter);
        title->setStyleSheet("font-size: 20px; font-weight: bold;");
        mainLayout->addWidget(title);

        statusLabel = new QLabel("Выключен");
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setStyleSheet("font-size: 14px; color: gray;");
        mainLayout->addWidget(statusLabel);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *onBtn = new QPushButton("Вкл");
        onBtn->setStyleSheet("background-color: green; color: white;");
        QPushButton *offBtn = new QPushButton("Выкл");
        offBtn->setStyleSheet("background-color: red; color: white;");
        QPushButton *sosBtn = new QPushButton("SOS");
        sosBtn->setStyleSheet("background-color: orange;");
        QPushButton *strobeBtn = new QPushButton("Строб");
        strobeBtn->setStyleSheet("background-color: purple; color: white;");
        QPushButton *fadeBtn = new QPushButton("Затухание");
        fadeBtn->setStyleSheet("background-color: blue; color: white;");
        btnLayout->addWidget(onBtn);
        btnLayout->addWidget(offBtn);
        btnLayout->addWidget(sosBtn);
        btnLayout->addWidget(strobeBtn);
        btnLayout->addWidget(fadeBtn);
        mainLayout->addLayout(btnLayout);

        QHBoxLayout *brightLayout = new QHBoxLayout();
        brightLayout->addWidget(new QLabel("Яркость:"));
        brightnessSlider = new QSlider(Qt::Horizontal);
        brightnessSlider->setRange(0, 100);
        brightnessSlider->setValue(brightness);
        connect(brightnessSlider, &QSlider::valueChanged, this, &Flashlight::setBrightness);
        brightLayout->addWidget(brightnessSlider);
        brightnessLabel = new QLabel(QString::number(brightness) + "%");
        brightLayout->addWidget(brightnessLabel);
        mainLayout->addLayout(brightLayout);

        indicator = new QLabel;
        indicator->setFixedSize(60, 60);
        indicator->setStyleSheet("background-color: gray; border-radius: 30px;");
        indicator->setAlignment(Qt::AlignCenter);
        mainLayout->addWidget(indicator, 0, Qt::AlignCenter);

        QLabel *status = new QLabel("Готов");
        mainLayout->addWidget(status);

        connect(onBtn, &QPushButton::clicked, this, &Flashlight::turnOn);
        connect(offBtn, &QPushButton::clicked, this, &Flashlight::turnOff);
        connect(sosBtn, &QPushButton::clicked, this, &Flashlight::setSOS);
        connect(strobeBtn, &QPushButton::clicked, this, &Flashlight::setStrobe);
        connect(fadeBtn, &QPushButton::clicked, this, &Flashlight::setFade);

        timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &Flashlight::updateIndicator);
    }

    void updateIndicator() {
        if (mode == "on" && isOn) {
            int intensity = 255 * brightness / 100;
            QString color = QString("#%1%2%3").arg(intensity, 2, 16, QChar('0'))
                                              .arg(intensity, 2, 16, QChar('0'))
                                              .arg(intensity, 2, 16, QChar('0'));
            indicator->setStyleSheet(QString("background-color: %1; border-radius: 30px;").arg(color));
        } else if (mode == "sos" || mode == "strobe" || mode == "fade") {
            // Цвет обновляется в циклах
        } else {
            indicator->setStyleSheet("background-color: gray; border-radius: 30px;");
        }
    }

    void startSOS() {
        stopFlag = false;
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            delete workerThread;
        }
        workerThread = QThread::create([this]() {
            while (running && !stopFlag) {
                for (int i = 0; i < 3 && !stopFlag; ++i) {
                    flash(200);
                    QThread::msleep(200);
                }
                for (int i = 0; i < 3 && !stopFlag; ++i) {
                    flash(600);
                    QThread::msleep(200);
                }
                for (int i = 0; i < 3 && !stopFlag; ++i) {
                    flash(200);
                    QThread::msleep(200);
                }
                QThread::msleep(800);
            }
        });
        workerThread->start();
    }

    void flash(int ms) {
        QMetaObject::invokeMethod(this, [this]() {
            indicator->setStyleSheet("background-color: yellow; border-radius: 30px;");
        });
        QThread::msleep(ms);
        if (!stopFlag) {
            QMetaObject::invokeMethod(this, [this]() {
                indicator->setStyleSheet("background-color: gray; border-radius: 30px;");
            });
        }
    }

    void startStrobe() {
        stopFlag = false;
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            delete workerThread;
        }
        workerThread = QThread::create([this]() {
            while (running && !stopFlag) {
                QMetaObject::invokeMethod(this, [this]() {
                    indicator->setStyleSheet("background-color: white; border-radius: 30px;");
                });
                QThread::msleep(50);
                if (stopFlag) break;
                QMetaObject::invokeMethod(this, [this]() {
                    indicator->setStyleSheet("background-color: gray; border-radius: 30px;");
                });
                QThread::msleep(50);
            }
        });
        workerThread->start();
    }

    void startFade() {
        stopFlag = false;
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            delete workerThread;
        }
        workerThread = QThread::create([this]() {
            while (running && !stopFlag) {
                for (int i = 0; i <= 100 && !stopFlag; i += 5) {
                    int intensity = 255 * i / 100;
                    QString color = QString("#%1%2%3").arg(intensity, 2, 16, QChar('0'))
                                                      .arg(intensity, 2, 16, QChar('0'))
                                                      .arg(intensity, 2, 16, QChar('0'));
                    QMetaObject::invokeMethod(this, [this, color]() {
                        indicator->setStyleSheet(QString("background-color: %1; border-radius: 30px;").arg(color));
                    });
                    QThread::msleep(50);
                }
                for (int i = 100; i >= 0 && !stopFlag; i -= 5) {
                    int intensity = 255 * i / 100;
                    QString color = QString("#%1%2%3").arg(intensity, 2, 16, QChar('0'))
                                                      .arg(intensity, 2, 16, QChar('0'))
                                                      .arg(intensity, 2, 16, QChar('0'));
                    QMetaObject::invokeMethod(this, [this, color]() {
                        indicator->setStyleSheet(QString("background-color: %1; border-radius: 30px;").arg(color));
                    });
                    QThread::msleep(50);
                }
            }
        });
        workerThread->start();
    }

    void stopCurrentMode() {
        running = false;
        stopFlag = true;
        if (workerThread) {
            workerThread->quit();
            workerThread->wait();
            delete workerThread;
            workerThread = nullptr;
        }
    }

    void loadSettings() {
        QSettings settings("MyApp", "FlashlightPro");
        brightness = settings.value("brightness", 80).toInt();
        mode = settings.value("mode", "off").toString();
        brightnessSlider->setValue(brightness);
        brightnessLabel->setText(QString::number(brightness) + "%");
        if (mode == "on") turnOn();
        else if (mode == "sos") setSOS();
        else if (mode == "strobe") setStrobe();
        else if (mode == "fade") setFade();
        else turnOff();
    }

    void saveSettings() {
        QSettings settings("MyApp", "FlashlightPro");
        settings.setValue("brightness", brightness);
        settings.setValue("mode", mode);
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    Flashlight w;
    w.show();
    return app.exec();
}

#include "flashlight_cpp.moc"
