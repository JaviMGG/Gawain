// main_window.h
#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QCheckBox>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    // Campos de texto
    QLineEdit *campoApp;
    QLineEdit *campoContraseña;

    // Botones de acción principales
    QPushButton *botonGenerar;
    QPushButton *botonListar;
    QPushButton *botonBuscar;
    QPushButton *botonEliminar;

    // Área de resultados
    QTextEdit *areaResultados;

    // Botones secundarios y de salida
    QPushButton *botonCopiar;
    QPushButton *botonLimpiar;
    QPushButton *botonSalir;

    // Checkbox seguro: si está marcado, el botón Generar queda deshabilitado
    QCheckBox *botonSeguro;
};

#endif // MAIN_WINDOW_H