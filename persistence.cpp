#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include "gawain.h"
#include "cifrado.h"

using namespace std;

bool guardar(const unsigned char clave[32], const unsigned char salt[16], unsigned long long opslimit, size_t memlimit)
{
    string textoPlano;
    for (auto &&elemento : gestionador)
    {
        textoPlano += elemento.first + "|" + elemento.second + "\n";
    }

    string textoCifrado = cifrarArchivo(textoPlano, clave, salt, opslimit, memlimit);

    if (textoCifrado == "")
    {
        return false;
    }

    ofstream salida("archivo.txt");
    
    if (!salida)
    {
        return false;
    }
    salida << textoCifrado << endl;
    chmod("archivo.txt", S_IRUSR | S_IWUSR);
    return true;
}

bool cargar(const unsigned char clave[32])
{
    ifstream entrada("archivo.txt");
    if (!entrada)
    {
        return true;    // no hay archivo: nada que cargar
    }

    string contenido;
    getline(entrada, contenido);

    string textoPlano;
    if (!descifrarArchivo(contenido, clave, textoPlano))
    {
        return false;
    }

    stringstream flujoTexto(textoPlano);
    string linea;
    while (getline(flujoTexto, linea))
    {
        if (linea.empty())
        {
            continue;
        }
        stringstream flujo(linea);
        string app;
        getline(flujo, app, '|');

        if (linea.find('|') == string::npos)
        {
            // Formato antiguo (sin separador): la contraseña generada
            // ocupa siempre 40 caracteres al final de la línea.
            const size_t LARGO_GENERADA = 40;
            if (linea.size() <= LARGO_GENERADA)
            {
                continue;
            }
            string contraseñaVieja = linea.substr(linea.size() - LARGO_GENERADA);
            string appVieja = linea.substr(0, linea.size() - LARGO_GENERADA);
            añadirContraseñas(appVieja, contraseñaVieja);
            continue;
        }

        string contraseña;
        while (getline(flujo, contraseña, '|'))
        {
            añadirContraseñas(app, contraseña);
        }
    }
    return true;
}
