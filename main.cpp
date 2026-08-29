#include <iostream>
#include "gawain.h"
#include "cifrado.h"
#include <fstream>
#include <sstream>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include "main_window.h"

using namespace std;
unsigned char salt[16];
unsigned char clave[32];
unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

static string pedirContrasena(const QString &titulo, const QString &mensaje)
{
    QDialog dialogo(nullptr);
    dialogo.setWindowTitle(titulo);
    dialogo.setFixedWidth(360);
    dialogo.setWindowIcon(QIcon(":/logo.png"));
    dialogo.setAutoFillBackground(true);

    QPalette pal = dialogo.palette();
    pal.setColor(QPalette::Window, QColor("#0f0d0b"));
    pal.setColor(QPalette::WindowText, QColor("#ece4d8"));
    dialogo.setPalette(pal);

    QVBoxLayout *layout = new QVBoxLayout(&dialogo);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    // Logo
    QLabel *etiquetaLogo = new QLabel;
    etiquetaLogo->setPixmap(QPixmap(":/logo.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    etiquetaLogo->setAlignment(Qt::AlignCenter);
    layout->addWidget(etiquetaLogo);

    // Separador
    QFrame *separador = new QFrame;
    separador->setFrameShape(QFrame::HLine);
    separador->setStyleSheet("background-color: #3d3020; max-height: 1px;");
    layout->addWidget(separador);

    // Mensaje
    QLabel *etiquetaMensaje = new QLabel(mensaje);
    etiquetaMensaje->setAlignment(Qt::AlignCenter);
    layout->addWidget(etiquetaMensaje);

    // Campo de contraseña
    QLineEdit *campo = new QLineEdit;
    campo->setEchoMode(QLineEdit::Password);
    campo->setPlaceholderText("Contraseña...");
    layout->addWidget(campo);

    // Botones
    QHBoxLayout *filaBotones = new QHBoxLayout;
    filaBotones->addStretch();
    QPushButton *botonCancelar = new QPushButton("Cancelar");
    QPushButton *botonAceptar = new QPushButton("Aceptar");
    filaBotones->addWidget(botonCancelar);
    filaBotones->addWidget(botonAceptar);
    layout->addLayout(filaBotones);

    QObject::connect(botonCancelar, &QPushButton::clicked, &dialogo, &QDialog::reject);
    QObject::connect(botonAceptar, &QPushButton::clicked, &dialogo, &QDialog::accept);
    QObject::connect(campo, &QLineEdit::returnPressed, &dialogo, &QDialog::accept);

    campo->setFocus();

    if (dialogo.exec() == QDialog::Accepted)
        return campo->text().toStdString();

    return "";
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // --- TEMA OSCURO ---
    app.setStyleSheet(R"(
        QMainWindow, QWidget {
            background-color: #0f0d0b;
            color: #ece4d8;
            font-family: 'Segoe UI', Helvetica, sans-serif;
            font-size: 14px;
        }
        QLineEdit {
            background-color: #191512;
            border: 1px solid #3d3020;
            border-radius: 6px;
            padding: 8px;
            color: #ffffff;
            selection-background-color: #6f4b1a;
        }
        QLineEdit:focus {
            border: 1px solid #e99a2d;
        }
        QTextEdit {
            background-color: #120f0c;
            border: 1px solid #3d3020;
            border-radius: 6px;
            padding: 10px;
            color: #e0b047;
            font-family: 'Consolas', monospace;
        }
        QPushButton {
            background-color: #251d11;
            border: 1px solid #45351c;
            border-radius: 6px;
            padding: 8px 16px;
            color: #ffffff;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #33280f;
            border: 1px solid #ba601c;
        }
        QPushButton:pressed {
            background-color: #e99a2d;
            border: 1px solid #e99a2d;
            color: #0b0a0a;
        }
        QPushButton:disabled {
            background-color: #171310;
            border: 1px solid #2c2317;
            color: #6b5f4d;
        }
        QGroupBox {
            border: 1px solid #3d3020;
            border-radius: 8px;
            margin-top: 14px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 5px;
            color: #b39b74;
        }
        #botonSalir {
            background-color: #7e2410;
            border: 1px solid #a33413;
        }
        #botonSalir:hover {
            background-color: #a33413;
        }
        #botonSalir:pressed {
            background-color: #d0430f;
        }
        QLabel {
            color: #ece4d8;
        }
        QMessageBox {
            background-color: #0f0d0b;
        }
    )");

    ifstream entrada("archivo.txt");
    string contenido;
    if (entrada)
    {
        getline(entrada, contenido);
    }

    string pw;

    if (!extraerSaltDeContenido(contenido, salt))
    {
        generarSalt(salt);
        opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
        memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

        string contraseñaM1, contraseñaM2;
        bool confirmada = false;

        while (!confirmada)
        {
            contraseñaM1 = pedirContrasena(
                "Gawain - Nueva contraseña",
                "Introduce la contraseña maestra:");

            if (contraseñaM1.empty()) return 0;

            contraseñaM2 = pedirContrasena(
                "Gawain - Confirmar contraseña",
                "Confirma la contraseña maestra:");

            if (contraseñaM2.empty()) return 0;

            if (contraseñaM1 == contraseñaM2)
            {
                confirmada = true;
            }
            else
            {
                QMessageBox::warning(
                    nullptr,
                    "Error",
                    "Las contraseñas no coinciden. Inténtalo de nuevo.");
            }
        }

        pw = contraseñaM1;
    }
    else
    {
        extraerParametrosDeContenido(contenido, opslimit, memlimit);

        pw = pedirContrasena(
            "Gawain - Contraseña maestra",
            "Introduce la contraseña maestra:");

        if (pw.empty()) return 0;
    }

    if (!derivarClave(pw, salt, clave, opslimit, memlimit))
    {
        QMessageBox::critical(nullptr, "Error", "No se pudo derivar la clave.");
        return 1;
    }
    
    if (!pw.empty())
        sodium_memzero(&pw[0], pw.size());
    pw.clear();
    
    if (!cargar(clave))
    {
        QMessageBox::warning(
            nullptr,
            "Error",
            "Contraseña maestra incorrecta.");
        return 1;
    }

    MainWindow ventana;
    ventana.show();
    int resultado = app.exec();

    sodium_memzero(clave, sizeof(clave));
    return resultado;
}
