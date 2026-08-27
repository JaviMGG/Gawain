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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Gawain - Gestor de Contraseñas");
    setWindowIcon(QIcon(":/logo.png"));
    resize(600, 500);
    setMinimumSize(550, 450);

    // --- INSTANCIACIÓN DE WIDGETS ---

    // Campos
    campoApp = new QLineEdit;
    campoApp->setPlaceholderText("Ej. Gmail, GitHub, Banco...");

    // Botones principales
    botonGenerar = new QPushButton("Generar");
    botonBuscar = new QPushButton("Buscar");
    botonListar = new QPushButton("Listar");
    botonEliminar = new QPushButton("Eliminar");

    // Área de resultados
    areaResultados = new QTextEdit;
    areaResultados->setReadOnly(true);
    areaResultados->setPlaceholderText("Resultados o contraseñas generadas aparecerán aquí...");

    // Botones secundarios
    botonCopiar = new QPushButton("Copiar al portapapeles");
    botonLimpiar = new QPushButton("Limpiar");
    botonSeguro = new QCheckBox("Modo seguro");
    botonSalir = new QPushButton("Salir");
    botonSalir->setObjectName("botonSalir"); // Identificador para el estilo rojo

    // --- DISEÑO DE LA INTERFAZ (LAYOUTS) ---

    // Logo de la app (pequeño, arriba a la izquierda)
    QLabel *etiquetaLogo = new QLabel;
    etiquetaLogo->setPixmap(QPixmap(":/logo.png").scaled(48, 48, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    QHBoxLayout *filaLogo = new QHBoxLayout;
    filaLogo->addWidget(etiquetaLogo);
    filaLogo->addStretch();

    // Contenedor principal
    QWidget *contenedor = new QWidget;
    QVBoxLayout *layoutPrincipal = new QVBoxLayout(contenedor);
    layoutPrincipal->setContentsMargins(20, 20, 20, 20);
    layoutPrincipal->setSpacing(15);

    // 1. Grupo del Formulario
    QGroupBox *grupoDatos = new QGroupBox("Datos de Entrada");
    QFormLayout *formulario = new QFormLayout(grupoDatos);
    formulario->setSpacing(12);
    formulario->addRow("App:", campoApp);
    
    // 2. Fila de botones de acción
    QHBoxLayout *filaAcciones = new QHBoxLayout;
    filaAcciones->setSpacing(10);
    filaAcciones->addWidget(botonGenerar);
    filaAcciones->addWidget(botonBuscar);
    filaAcciones->addWidget(botonListar);
    filaAcciones->addWidget(botonEliminar);

    // 3. Fila de botones inferiores (Utilidades y Salir)
    QHBoxLayout *filaInferior = new QHBoxLayout;
    filaInferior->addWidget(botonCopiar);
    filaInferior->addWidget(botonLimpiar);
    filaInferior->addWidget(botonSeguro);
    filaInferior->addStretch(); // Empuja "Salir" a la derecha
    filaInferior->addWidget(botonSalir);

    // Ensamblaje
    layoutPrincipal->addLayout(filaLogo);
    layoutPrincipal->addWidget(grupoDatos);
    layoutPrincipal->addLayout(filaAcciones);
    layoutPrincipal->addWidget(areaResultados, 1); // El área de texto toma el espacio extra
    layoutPrincipal->addLayout(filaInferior);

    setCentralWidget(contenedor);

    //Buscar
    connect(botonBuscar, &QPushButton::clicked, this, [this]{
        string app = campoApp->text().toStdString();
        string pw = buscarContraseña(campoApp->text().toStdString());
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }
        areaResultados->setPlainText(QString::fromStdString(
            pw));
    });

    //Listar
    connect(botonListar, &QPushButton::clicked, this, [this] {
        areaResultados->setPlainText(QString::fromStdString(listarTodo()));
    });

    //Generar
    connect(botonGenerar, &QPushButton::clicked, this, [this] {
        string app = campoApp->text().toStdString();
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }
        string cont = generarContraseña();
        añadirContraseñas(app, cont);
        guardar(clave, salt, opslimit, memlimit);
        areaResultados->setPlainText(QString::fromStdString(
            "Contraseña generada para " + app + ":\n" + cont));
    });

    //Salir
    connect(botonSalir, &QPushButton::clicked, this, &QWidget::close);

    //Limpiar: borra campos/resultados y re-habilita Generar
    connect(botonLimpiar, &QPushButton::clicked, this, [this] {
        campoApp->clear();
        areaResultados->clear();
        botonGenerar->setEnabled(true);
    });

    //Modo seguro: al pulsarlo Generar se deshabilita; al desactivarlo vuelve a habilitarse
    connect(botonSeguro, &QCheckBox::toggled, this, [this](bool checked) {
        botonGenerar->setEnabled(!checked);
    });

    //Eliminar: al clicarlo, borra la app (clave y contraseña) y guarda los cambios
    connect(botonEliminar, &QPushButton::clicked, this, [this]{
        string app = campoApp->text().toStdString();
        if (app.empty())
        {
            areaResultados->setPlainText("Escribe el nombre de la app.");
            return;
        }

        if (!eliminarContraseña(app))
        {
            areaResultados->setPlainText(QString::fromStdString("La app \"" + app + "\" no existe."));
            return;
        }

        guardar(clave, salt, opslimit, memlimit);
        areaResultados->setPlainText(QString::fromStdString("Entrada de \"" + app + "\" eliminada."));
    });

    //Copiar
    connect(botonCopiar, &QPushButton::clicked, this, [this]{
        string textoApp = campoApp->text().toStdString();
        if (textoApp.empty()) {
            areaResultados->setPlainText("Por favor escribe el nombre de la app para copiar.");
            return;
        }
        if (gestionador.count(textoApp) == 0)
        {
            areaResultados->setPlainText("El nombre de la app no se encuentra.\n\nApps guardadas:\n" + QString::fromStdString(listarTodo()));
            return;
        }
        
        botonBuscar->click();
        QString texto = areaResultados->toPlainText();
        QApplication::clipboard()->setText(texto);
        areaResultados->setPlainText("Contraseña copiada al portapapeles.");
        
        //limpiar portapapeles
        QTimer::singleShot(15000, this, []{  // 15 segundos
        QApplication::clipboard()->clear();
        });
    });

    //Atajos de teclado
    new QShortcut(QKeySequence("Ctrl+C"), this, [this]{
        botonCopiar->click();
    });

    new QShortcut(QKeySequence("Ctrl+F"), this, [this]{
        botonBuscar->click();
    });

    new QShortcut(QKeySequence("Ctrl+X"), this, [this]{
        botonLimpiar->click();
    });

    new QShortcut(QKeySequence("Ctrl+L"), this, [this]{
        botonListar->click();
    });

    new QShortcut(QKeySequence("Ctrl+D"), this, [this]{
        botonEliminar->click();
    });

    new QShortcut(QKeySequence("Ctrl+S"), this, [this]{
        botonSeguro->click();
    });

    new QShortcut(QKeySequence("Ctrl+Q"), this, [this]{
        botonSalir->click();
    });
}