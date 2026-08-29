#include "main_window.h"
#include "gawain.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QShortcut>
#include <QKeySequence>

extern unsigned char salt[16];
extern unsigned char clave[32];
extern unsigned long long opslimit;
extern size_t memlimit;

MainWindow::MainWindow(Vault &vault, QWidget *parent)
    : QMainWindow(parent), vault(vault)
{
    setWindowTitle("Gawain - Gestor de Contraseñas");
    setWindowIcon(QIcon(":/logo.png"));
    resize(600, 500);
    setMinimumSize(550, 450);

    campoApp = new QLineEdit;
    campoApp->setPlaceholderText("Ej. Gmail, GitHub, Banco...");

    botonGenerar = new QPushButton("Generar");
    botonBuscar = new QPushButton("Buscar");
    botonListar = new QPushButton("Listar");
    botonEliminar = new QPushButton("Eliminar");

    areaResultados = new QTextEdit;
    areaResultados->setReadOnly(true);
    areaResultados->setPlaceholderText("Resultados o contraseñas generadas aparecerán aquí...");

    botonCopiar = new QPushButton("Copiar al portapapeles");
    botonLimpiar = new QPushButton("Limpiar");
    botonSeguro = new QCheckBox("Modo seguro");
    botonSalir = new QPushButton("Salir");
    botonSalir->setObjectName("botonSalir");

    QLabel *etiquetaLogo = new QLabel;
    etiquetaLogo->setPixmap(QPixmap(":/logo.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QHBoxLayout *filaLogo = new QHBoxLayout;
    filaLogo->addWidget(etiquetaLogo);
    filaLogo->addStretch();

    QWidget *contenedor = new QWidget;
    QVBoxLayout *layoutPrincipal = new QVBoxLayout(contenedor);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    layoutPrincipal->setSpacing(15);

    QGroupBox *grupoDatos = new QGroupBox("Datos de Entrada");
    QFormLayout *formulario = new QFormLayout(grupoDatos);
    formulario->setSpacing(12);
    formulario->addRow("App:", campoApp);

    QHBoxLayout *filaAcciones = new QHBoxLayout;
    filaAcciones->setSpacing(10);
    filaAcciones->addWidget(botonGenerar);
    filaAcciones->addWidget(botonBuscar);
    filaAcciones->addWidget(botonListar);
    filaAcciones->addWidget(botonEliminar);

    QHBoxLayout *filaInferior = new QHBoxLayout;
    filaInferior->addWidget(botonCopiar);
    filaInferior->addWidget(botonLimpiar);
    filaInferior->addWidget(botonSeguro);
    filaInferior->addStretch();
    filaInferior->addWidget(botonSalir);

    layoutPrincipal->addLayout(filaLogo);
    layoutPrincipal->addWidget(grupoDatos);
    layoutPrincipal->addLayout(filaAcciones);
    layoutPrincipal->addWidget(areaResultados, 1);
    layoutPrincipal->addLayout(filaInferior);

    setCentralWidget(contenedor);

    connect(botonBuscar, &QPushButton::clicked, this, [this, &vault]{
        string app = campoApp->text().toStdString();
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }
        string pw = vault.buscarContraseña(app);
        areaResultados->setPlainText(QString::fromStdString(pw));
    });

    connect(botonListar, &QPushButton::clicked, this, [this, &vault] {
        areaResultados->setPlainText(QString::fromStdString(vault.listarTodo()));
    });

    connect(botonGenerar, &QPushButton::clicked, this, [this, &vault] {
        string app = campoApp->text().toStdString();
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }
        if (app.find('|') != string::npos ||
            app.find('\n') != string::npos ||
            app.find('\r') != string::npos)
        {
            areaResultados->setPlainText("El nombre de la app no puede contener | o saltos de línea.");
            return;
        }
        string cont = vault.generarContraseña();
        vault.añadirContraseñas(app, cont);
        if(!guardar(clave, salt, opslimit, memlimit))
        {
            areaResultados->setPlainText("No se ha podido guardar correctamente.");
        }
        else
        {
            areaResultados->setPlainText(QString::fromStdString("Contraseña generada para " + app + ":\n" + cont));
        }
    });

    connect(botonSalir, &QPushButton::clicked, this, &QWidget::close);

    connect(botonLimpiar, &QPushButton::clicked, this, [this] {
        campoApp->clear();
        areaResultados->clear();
        botonGenerar->setEnabled(true);
    });

    connect(botonSeguro, &QCheckBox::toggled, this, [this](bool checked) {
        botonGenerar->setEnabled(!checked);
    });

    connect(botonEliminar, &QPushButton::clicked, this, [this, &vault]{
        string app = campoApp->text().toStdString();
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }
        if (app.find('|') != string::npos ||
            app.find('\n') != string::npos ||
            app.find('\r') != string::npos)
        {
            areaResultados->setPlainText("El nombre de la app no puede contener | o saltos de línea.");
            return;
        }

        if (!vault.eliminarContraseña(app))
        {
            areaResultados->setPlainText(QString::fromStdString("La app \"" + app + "\" no existe."));
            return;
        }

        if (guardar(clave, salt, opslimit, memlimit))
        {
            areaResultados->setPlainText(QString::fromStdString("Entrada de \"" + app + "\" eliminada."));
        }
        else
        {
            areaResultados->setPlainText(QString::fromStdString("Entrada de \"" + app + "\" no se ha podido eliminar."));
        }
    });

    connect(botonCopiar, &QPushButton::clicked, this, [this, &vault]{
        string textoApp = campoApp->text().toStdString();
        if (textoApp.empty()) {
            areaResultados->setPlainText("Por favor escribe el nombre de la app para copiar.");
            return;
        }
        if (vault.buscarContraseña(textoApp).empty())
        {
            areaResultados->setPlainText("El nombre de la app no se encuentra.\n\nApps guardadas:\n" + QString::fromStdString(vault.listarTodo()));
            return;
        }

        botonBuscar->click();
        QString texto = areaResultados->toPlainText();
        QApplication::clipboard()->setText(texto);
        areaResultados->setPlainText("Contraseña copiada al portapapeles.");

        QTimer::singleShot(15000, this, []{
            QApplication::clipboard()->clear();
        });
    });

    new QShortcut(QKeySequence("Ctrl+Shift+C"), this, [this]{ botonCopiar->click(); });
    new QShortcut(QKeySequence("Ctrl+F"), this, [this]{ botonBuscar->click(); });
    new QShortcut(QKeySequence("Ctrl+X"), this, [this]{ botonLimpiar->click(); });
    new QShortcut(QKeySequence("Ctrl+L"), this, [this]{ botonListar->click(); });
    new QShortcut(QKeySequence("Ctrl+D"), this, [this]{ botonEliminar->click(); });
    new QShortcut(QKeySequence("Ctrl+S"), this, [this]{ botonSeguro->click(); });
    new QShortcut(QKeySequence("Ctrl+Q"), this, [this]{ botonSalir->click(); });
}
