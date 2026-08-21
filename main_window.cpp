#include "main_window.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Gawain - Gestor de Contraseñas");
    setWindowIcon(QIcon(":/logo.png"));
    resize(600, 500);
    setMinimumSize(550, 450);

    // --- TEMA OSCURO (paleta extraída del logo: negros + ámbar/dorado/marrones) ---
    setStyleSheet(R"(
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
            color: #e0b047; /* dorado del logo */
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
    )");

    // --- INSTANCIACIÓN DE WIDGETS ---

    // Campos
    campoApp = new QLineEdit;
    campoApp->setPlaceholderText("Ej. Gmail, GitHub, Banco...");

    campoContraseña = new QLineEdit;
    campoContraseña->setPlaceholderText("Contraseña...");

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
    formulario->addRow("Contraseña:", campoContraseña);

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
    filaInferior->addStretch(); // Empuja "Salir" a la derecha
    filaInferior->addWidget(botonSalir);

    // Ensamblaje
    layoutPrincipal->addLayout(filaLogo);
    layoutPrincipal->addWidget(grupoDatos);
    layoutPrincipal->addLayout(filaAcciones);
    layoutPrincipal->addWidget(areaResultados, 1); // El área de texto toma el espacio extra
    layoutPrincipal->addLayout(filaInferior);

    setCentralWidget(contenedor);

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

    
}