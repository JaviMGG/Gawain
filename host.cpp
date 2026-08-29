#include <iostream>
#include <string>
#include <fstream>
#include <sodium.h>
#include <QApplication>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include "gawain.h"
#include "cifrado.h"

using namespace std;

static string pedirContrasena(const QString &titulo, const QString &mensaje)
{
    QDialog dialogo(nullptr);
    dialogo.setWindowTitle(titulo);
    dialogo.setFixedWidth(360);
    dialogo.setAutoFillBackground(true);

    QPalette pal = dialogo.palette();
    pal.setColor(QPalette::Window, QColor("#0f0d0b"));
    pal.setColor(QPalette::WindowText, QColor("#ece4d8"));
    dialogo.setPalette(pal);

    QVBoxLayout *layout = new QVBoxLayout(&dialogo);
    layout->setContentsMargins(24, 20, 24, 20);
    layout->setSpacing(12);

    QFrame *separador = new QFrame;
    separador->setFrameShape(QFrame::HLine);
    separador->setStyleSheet("background-color: #3d3020; max-height: 1px;");
    layout->addWidget(separador);

    QLabel *etiquetaMensaje = new QLabel(mensaje);
    etiquetaMensaje->setAlignment(Qt::AlignCenter);
    layout->addWidget(etiquetaMensaje);

    QLineEdit *campo = new QLineEdit;
    campo->setEchoMode(QLineEdit::Password);
    campo->setPlaceholderText("Contraseña...");
    layout->addWidget(campo);

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

static void responder(const QJsonObject &obj)
{
    QJsonDocument doc(obj);
    cout << doc.toJson(QJsonDocument::Compact).toStdString() << endl;
}

static bool esNombreValido(const string &app)
{
    return app.find('|') == string::npos &&
           app.find('\n') == string::npos &&
           app.find('\r') == string::npos;
}

static bool desbloquear(unsigned char *clave, unsigned char *salt,
                         unsigned long long &opslimit, size_t &memlimit)
{
    ifstream entrada(rutaBaseDatos());
    string contenido;
    if (entrada)
        getline(entrada, contenido);

    if (!extraerSaltDeContenido(contenido, salt))
    {
        generarSalt(salt);
        opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
        memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

        string m1 = pedirContrasena("Gawain", "Nueva contraseña maestra:");
        if (m1.empty()) return false;

        string m2 = pedirContrasena("Gawain", "Confirma la contraseña maestra:");
        if (m2.empty()) return false;

        if (m1 != m2)
        {
            QMessageBox::warning(nullptr, "Error", "Las contraseñas no coinciden.");
            return false;
        }

        bool ok = derivarClave(m1, salt, clave, opslimit, memlimit);
        sodium_memzero(&m1[0], m1.size());
        sodium_memzero(&m2[0], m2.size());
        return ok;
    }

    extraerParametrosDeContenido(contenido, opslimit, memlimit);

    string m = pedirContrasena("Gawain", "Contraseña maestra:");
    if (m.empty()) return false;

    bool ok = derivarClave(m, salt, clave, opslimit, memlimit);
    sodium_memzero(&m[0], m.size());
    return ok;
}

static void procesar(const QJsonObject &msg)
{
    QString op = msg["op"].toString();

    if (op == "ping")
    {
        responder({{"ok", true}});
        return;
    }
    if (op == "quit")
    {
        exit(0);
    }

    unsigned char salt[16], clave[32];
    unsigned long long opslimit;
    size_t memlimit;

    if (!desbloquear(clave, salt, opslimit, memlimit))
    {
        responder({{"ok", false}, {"error", "cancelado"}});
        return;
    }

    gestionador.clear();
    if (!cargar(clave))
    {
        sodium_memzero(clave, sizeof(clave));
        responder({{"ok", false}, {"error", "contraseña incorrecta"}});
        return;
    }

    Vault v;

    if (op == "generate")
    {
        string pw = v.generarContraseña();
        sodium_memzero(clave, sizeof(clave));
        responder({{"ok", true}, {"password", QString::fromStdString(pw)}});
    }
    else if (op == "save")
    {
        string app = msg["app"].toString().toStdString();
        string password = msg["password"].toString().toStdString();

        if (app.empty() || password.empty())
        {
            sodium_memzero(clave, sizeof(clave));
            responder({{"ok", false}, {"error", "app y password requeridos"}});
            return;
        }
        if (!esNombreValido(app))
        {
            sodium_memzero(clave, sizeof(clave));
            responder({{"ok", false}, {"error", "nombre de app inválido"}});
            return;
        }

        v.añadirContraseñas(app, password);
        if (!guardar(clave, salt, opslimit, memlimit))
        {
            sodium_memzero(clave, sizeof(clave));
            responder({{"ok", false}, {"error", "error al guardar"}});
            return;
        }
        sodium_memzero(clave, sizeof(clave));
        responder({{"ok", true}});
    }
    else if (op == "get")
    {
        string app = msg["app"].toString().toStdString();
        string pw = v.buscarContraseña(app);
        sodium_memzero(clave, sizeof(clave));
        if (pw.empty())
        {
            responder({{"ok", false}, {"error", "app no encontrada"}});
        }
        else
        {
            responder({{"ok", true}, {"password", QString::fromStdString(pw)}});
        }
    }
    else
    {
        sodium_memzero(clave, sizeof(clave));
        responder({{"ok", false}, {"error", "operación desconocida"}});
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (sodium_init() < 0)
    {
        cerr << "Error al inicializar libsodium" << endl;
        return 1;
    }

    string linea;
    while (getline(cin, linea))
    {
        if (linea.empty()) continue;
        QJsonDocument doc = QJsonDocument::fromJson(QByteArray::fromStdString(linea));
        if (doc.isNull() || !doc.isObject())
        {
            responder({{"ok", false}, {"error", "JSON inválido"}});
            continue;
        }
        procesar(doc.object());
    }
    return 0;
}
