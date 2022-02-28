#include "MainWindow.h"

#include <QFileDialog>
//#include <QMessageBox>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
//#include <QApplication>

#include "EmuWindow.h"

MainWindow::MainWindow()
{
    m_emu_window = new EmuWindow(this);

    m_file_menu = menuBar()->addMenu("File");
    m_emulation_menu = menuBar()->addMenu("Emulation");

    m_open_file_action = m_file_menu->addAction("Open", this,&MainWindow::chooseFile,QKeySequence(QStringLiteral("Ctrl+O")));

    m_close_action = m_file_menu->addAction("Close", m_emu_window, &EmuWindow::stopEmulation);

    m_change_speed_menu = m_emulation_menu->addMenu("Change Emulation Speed");

    m_quit_action = new QAction("Quit", this);
    m_quit_action->setShortcut(QKeySequence("Alt+F4"));
    m_file_menu->addAction(m_quit_action);

    setCentralWidget(m_emu_window);
}

MainWindow::~MainWindow() = default;

void MainWindow::chooseFile()
{
    const std::string file_name = QFileDialog::getOpenFileName(this, "Ouvrir un fichier", QString(), "C8 Rom (*.c8 *.ch8);;All files (*.*)").toStdString();

    if(!file_name.empty())
    {
        m_emu_window->startEmulation(file_name.c_str());
    }
    //QMessageBox::warning(this, "Erreur", "Aucun fichier n'a été sélectionné !");
}

