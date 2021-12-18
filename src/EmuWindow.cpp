#include "EmuWindow.h"

#include <iostream>

#include <QTimer>
#include <QKeyEvent>
#include <QMessageBox>
#include "RenderWindow.h"

static std::map<Qt::Key,char> key_mapping = {
    {Qt::Key_X,'0'},
    {Qt::Key_Ampersand,'1'},
    {Qt::Key_Eacute,'2'},
    {Qt::Key_QuoteDbl,'3'},
    {Qt::Key_A,'4'},
    {Qt::Key_Z,'5'},
    {Qt::Key_E,'6'},
    {Qt::Key_Q,'7'},
    {Qt::Key_S,'8'},
    {Qt::Key_D,'9'},
    {Qt::Key_W,'A'},
    {Qt::Key_C,'B'},
    {Qt::Key_Apostrophe,'C'},
    {Qt::Key_R,'D'},
    {Qt::Key_F,'E'},
    {Qt::Key_V,'F'},

};

EmuWindow::EmuWindow(QWidget *parent) : QGraphicsView (parent)
{
    m_frame_timer = new QTimer(this);
    m_frame_timer->setInterval(1000/500);

    constexpr int delay_and_sound_frequency = 60;
    m_delay_and_sound_timer = new QTimer(this);
    m_delay_and_sound_timer->setInterval(1000/delay_and_sound_frequency);

    m_render_window = new RenderWindow();
    setScene(m_render_window);

    connectSignals();
}

void EmuWindow::connectSignals()
{
    QObject::connect(m_frame_timer, &QTimer::timeout, this, &EmuWindow::updateFrame);
    QObject::connect(m_delay_and_sound_timer, &QTimer::timeout, this, &EmuWindow::updateDelayAndSound);
}

void EmuWindow::startEmulation(const char* file_name)
{
    if(m_chip8.loadApplication(file_name) == Chip8::LOADING_SUCCESS)
    {
        m_is_emulation_running = true;
        m_frame_timer->start();
        m_delay_and_sound_timer->start();
    }
    else
    {
        QMessageBox::warning(this, "Erreur", "Le fichier spécifié n'a pas pu être ouvert");
    }
}

void EmuWindow::stopEmulation()
{
    m_is_emulation_running = false;
    m_frame_timer->stop();
    m_delay_and_sound_timer->stop();
    m_render_window->reset();
    m_chip8.reset();
}

void EmuWindow::updateFrame()
{
    m_chip8.emulateCycle();
    if(m_chip8.needToRedraw())
    {
        m_render_window->draw(m_chip8.getGFX());
    }
}

void EmuWindow::keyPressEvent(QKeyEvent *event) {
    if(m_is_emulation_running)
    {
        const auto key_pressed = static_cast<Qt::Key>(event->key());
        const auto corresponding_chip8_key = key_mapping.find(key_pressed);
        if(corresponding_chip8_key != key_mapping.end())
        {
            m_chip8.setKeyPressed(corresponding_chip8_key->second);
        }
    }
}

void EmuWindow::keyReleaseEvent(QKeyEvent *event) {
    if(m_is_emulation_running)
    {
        const auto key_pressed = static_cast<Qt::Key>(event->key());
        const auto corresponding_chip8_key = key_mapping.find(key_pressed);
        if(corresponding_chip8_key != key_mapping.end())
        {
            m_chip8.setKeyReleased(corresponding_chip8_key->second);
        }
    }
}

void EmuWindow::updateDelayAndSound()
{
    m_chip8.countDownDelayTimer();
    if(m_chip8.countDownSoundTimer() > 0)
    {
        std::cout << "BEEP !" << std::endl;
    }
}

void EmuWindow::setEmulationSpeed(const int frequency)
{
    if(m_frame_timer->isActive())
    {
        m_frame_timer->start(1000/frequency);
    }
    else
    {
        m_frame_timer->setInterval(1000/frequency);
    }
}
