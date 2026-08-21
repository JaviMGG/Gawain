#include <iostream>
#include "gawain.h"
#include "cifrado.h"
#include <fstream>
#include <sstream>
#include <termios.h>
#include <unistd.h>
#include <QApplication>
#include "main_window.h"

using namespace std;
unsigned char salt[16];
unsigned char clave[32];
unsigned long long opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
size_t memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

static string pedirPasswordOculta(const string &mensaje)
{
    cout << mensaje;
    termios atributos;
    tcgetattr(STDIN_FILENO, &atributos);
    termios sinEco = atributos;
    sinEco.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &sinEco);
    string password;
    getline(cin, password);
    tcsetattr(STDIN_FILENO, TCSANOW, &atributos);
    cout << endl;
    return password;
}

static void bienvenida(int opcion)
{
    cout << "1. Generar Contraseña" << endl;
    cout << "2. Listar Contraseña" << endl;
    cout << "3. Buscar Contraseña" << endl;
    cout << "4. Eliminar Contraseña" << endl;
    cout << "0. Salir" << endl;

    switch (opcion)
    {
    case 1:
    {
        string app;
        cout << "¿Para qué app?" << endl;
        cin >> app;
        string cont = generarContraseña();
        añadirContraseñas(app, cont);
        guardar(clave, salt, opslimit, memlimit);
        cout << "Contraseña generada para " << app << ": " << cont << endl;
        break;
    }
    case 2:
        cout << listarTodo();
        break;
    case 3:
    {
        string app;
        cout << "¿De qué app quieres ver la contraseña?" << endl;
        cin >> app;
        string r = buscarContraseña(app);
        if (r.empty())
        {
            cout << "Esta app no existe" << endl;
        }
        else
        {
            cout << r << endl;
        }
        break;
    }
    case 4:
    {
        string app;
        string contraseña;
        cout << "¿De qué app?" << endl;
        cin >> app;
        cout << "¿Qué contraseña quieres eliminar?" << endl;
        cin >> contraseña;
        if (eliminarContraseña(app, contraseña))
        {
            guardar(clave, salt, opslimit, memlimit);
            cout << "Eliminacion realizada con éxito" << endl;
        }
        break;
    }
    default:
        cout << "Opción no válida." << endl;
        break;
    }
}

int main(int argc, char *argv[])
{
    ifstream entrada("archivo.txt");
    string contenido;
    if (entrada)
    {
        getline(entrada, contenido);
    }

    string pw;

    if (!extraerSaltDeContenido(contenido, salt))
    {
        // Archivo nuevo: generar salt, confirmar contraseña 2 veces.
        generarSalt(salt);
        opslimit = crypto_pwhash_OPSLIMIT_MODERATE;
        memlimit = crypto_pwhash_MEMLIMIT_MODERATE;

        string contraseñaM1, contraseñaM2;
        do
        {
            contraseñaM1 = pedirPasswordOculta("Por favor, escribe la contraseña maestra:");
            contraseñaM2 = pedirPasswordOculta("Por favor, escribe la contraseña maestra de nuevo:");
        } while (contraseñaM1 != contraseñaM2);
        pw = contraseñaM1;
    }
    else
    {
        // Archivo existente: extraer opslimit y memlimit del contenido v2.
        extraerParametrosDeContenido(contenido, opslimit, memlimit);
        pw = pedirPasswordOculta("Por favor, escribe la contraseña maestra:");
    }

    derivarClave(pw, salt, clave, opslimit, memlimit);
    sodium_memzero(&pw[0], pw.size());

    if (!cargar(clave))
    {
        sleep(2);
        cout << "Contraseña maestra incorrecta." << endl;
        return 1;
    }

    QApplication app(argc, argv);
    MainWindow ventana;
    ventana.show();
    int resultado = app.exec();

    cout << "Adios" << endl;
    sodium_memzero(clave, sizeof(clave));
    return resultado;
}
