#include "gawain.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

const char elementosTeclado[] = "1234567890qwertyuioplkjhgfdsazxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ";

//diccionario con clave el nombre de la app y valor su contraseña
map<string, string> gestionador;

/**
 * Metodo que genera una contraseña.
 * @return string
*/
string generarContraseña()
{
    static mt19937 generador(random_device{}());
    static uniform_int_distribution<int> distribucion(0, sizeof(elementosTeclado) - 2);
    string contraseña;
    for (int i = 0; i < 40; i++)
    {
        contraseña += elementosTeclado[distribucion(generador)];
    }
    return contraseña;
}

/**
 * Metodo que coge las contraseñas guardadas y las devuelve en texto
 * @param vector
 * @return string
 */
string getContraseñasToString(const string &app)
{
    return gestionador[app];
}

/**
 * Metodo que coge todas las contraseñas guardadas y las devuelve en texto
 * @param vector
 * @return string
 */
string getContraseñasToStringConDelimitador(const string &app)
{
    return "|" + gestionador[app];
}

/**
 * Metodo que asocia una contraseña a una app.
 * @param string, string
*/
void añadirContraseñas(const string &app, const string &contraseña)
{
    gestionador[app] = contraseña;
}

/**
 * Metodo que devuelve todas las contraseñas de un vector.
 * @param string
 * @return string
*/
string listarContraseñas(const string &app)
{
    return gestionador[app];    
}

/**
 * Metodo que devuelve toda la informacion del gestor.
 * @return string
 */
string listarTodo()
{
    string todo;
    for (const auto &elemento : gestionador)
    {
        // Añadimos la información de cada elemento al string 'todo'
        todo += " App: " + elemento.first + ", Contraseña: " + elemento.second + "\n";
    }
    // Una vez terminado el bucle, devolvemos el string completo
    return todo;
}

string buscarContraseña(const string &app)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        return "";
    }
    // Devuelve el valor directamente usando el iterador
    return itApp->second; 
}

bool eliminarContraseña(const string &app, const string &contraseña)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        cout << "La app \"" << app << "\" no existe." << endl;
        return false;
    }

    gestionador[app].clear();
    if (gestionador[app].empty())
    {
        return true;
    }
    else
    {
        cout << "Esa contraseña no existe en la app \"" << app << "\"." << endl;
        return false;
    }
}
