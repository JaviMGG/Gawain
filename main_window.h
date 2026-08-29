// main_window.h
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>

class Vault;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(Vault &vault, QWidget *parent = nullptr);

private:
    Vault &vault;

    QLineEdit *campoApp;

    QPushButton *botonGenerar;
    QPushButton *botonListar;
    QPushButton *botonBuscar;
    QPushButton *botonEliminar;

    QTextEdit *areaResultados;

    QPushButton *botonCopiar;
    QPushButton *botonLimpiar;
    QPushButton *botonSalir;

    QCheckBox *botonSeguro;
};

#endif // MAIN_WINDOW_H
