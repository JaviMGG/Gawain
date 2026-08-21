#include "gawain.h"
#include <iostream>
#include <random>
#include <algorithm>

using namespace std;

const char elementosTeclado[] = "1234567890qwertyuioplkjhgfdsazxcvbnmABCDEFGHIJKLMNOPQRSTUVWXYZ";

map<string, vector<string>> gestionador;

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
 * Metodo que coge todas las contraseñas guardadas y las devuelve en texto
 * @param vector
 * @return string
 */
string getContraseñasToString(const vector<string> &contraseñas)
{
    string todo;
    for (const auto &contraseña : contraseñas)
    {
        todo += " " + contraseña;
    }
    return todo;
}

/**
 * Metodo que coge todas las contraseñas guardadas y las devuelve en texto
 * @param vector
 * @return string
 */
string getContraseñasToStringConDelimitador(const vector<string> &contraseñas)
{
    string todo;
    for (const auto &contraseña : contraseñas)
    {
        todo += "|" + contraseña;
    }
    return todo;
}

/**
 * Metodo que añade una contraseña al vector de contraseñas de una app.
 * @param string, string
*/
void añadirContraseñas(const string &app, const string &contraseña)
{
    gestionador[app].push_back(contraseña);
}

/**
 * Metodo que devuelve todas las contraseñas de un vector.
 * @param vector<string>
 * @return string
*/
string listarContraseñas(const vector<string> &contraseñas)
{
    string todas;
    for (const auto &contraseña : contraseñas)
    {
        todas += " " + contraseña;
    }
    return todas;
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
        todo += " App: " + elemento.first + ", Contraseñas: " + getContraseñasToString(elemento.second) + "\n";
    }
    return todo;
}

string buscarContraseña(const string &app)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        return "";
    }
    return listarContraseñas(itApp->second);
}

bool eliminarContraseña(const string &app, const string &contraseña)
{
    auto itApp = gestionador.find(app);
    if (itApp == gestionador.end())
    {
        cout << "La app \"" << app << "\" no existe." << endl;
        return false;
    }

    vector<string> &contraseñasApp = itApp->second;
    auto itContraseña = find(contraseñasApp.begin(), contraseñasApp.end(), contraseña);
    if (itContraseña != contraseñasApp.end())
    {
        contraseñasApp.erase(itContraseña);
        if (contraseñasApp.empty())
        {
            gestionador.erase(itApp);
        }
        return true;
    }
    else
    {
        cout << "Esa contraseña no existe en la app \"" << app << "\"." << endl;
        return false;
    }
}
