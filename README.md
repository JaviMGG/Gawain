<div align="center">
  <img src="logo.png" alt="Logo de Gawain" width="280"/>
</div>

# Gawain

Gestor de contraseñas de escritorio escrito en C++.

## Características

- Interfaz gráfica con **Qt 6**: tema oscuro inspirado en el logo, para generar, buscar, listar y eliminar contraseñas por aplicación
- **Modo seguro**: deshabilita la generación para consultas sin riesgo de sobrescribir nada
- Generación aleatoria de contraseñas (40 caracteres alfanuméricos)
- Almacenamiento cifrado con [libsodium](https://libsodium.org/)
  - Derivación de clave: **Argon2id** (MODERATE)
  - Cifrado autenticado: **AES-256-GCM**
- Formato v2: los parámetros de derivación (`opslimit`, `memlimit`) viajan en la cabecera del archivo, así el coste puede subir en el futuro sin romper bases de datos antiguas
- Contraseña maestra confirmada dos veces en la primera ejecución mediante diálogo gráfico personalizado con logo
- Clave y contraseña borradas de memoria al salir (`sodium_memzero`)
- Permisos `0600` en la base de datos
- **Host nativo** (`gawain-host`): puente para extensión de Firefox via Native Messaging, opera sobre la misma BD
- Ruta de BD unificada en `~/.gawain/archivo.txt` (compartida entre GUI y host)

## Fases del proyecto

### Fase 1 — Gestor en consola
La base del programa: un `map<string, string>` que asocia cada aplicación con su contraseña, y un menú por terminal para generar, listar, buscar y eliminar entradas.

### Fase 2 — Generador de contraseñas
Generador aleatorio basado en `<random>` (`mt19937` + `uniform_int_distribution`) sobre un alfabeto alfanumérico, devolviendo cadenas de 40 caracteres.

### Fase 3 — Cifrado real con libsodium
El paso de "archivo de texto plano" a "base de datos cifrada":
- Se separa el proyecto en módulos: lógica (`gawain.cpp/h`), persistencia (`persistence.cpp`) y cifrado (`cifrado.cpp/h`).
- La contraseña maestra no se usa directamente: se deriva una clave de 32 bytes con **Argon2id** y un salt aleatorio de 16 bytes.
- El contenido se cifra con **AES-256-GCM**, que además de cifrar firma los datos: si el archivo se manipula o la contraseña es incorrecta, el descifrado falla.
- La contraseña se introduce oculta mediante un diálogo gráfico personalizado (`QDialog` con logo, separador y `QLineEdit::Password`).

### Fase 4 — Endurecimiento
Cinco mejoras de seguridad sobre la Fase 3:
1. **Confirmar contraseña maestra** dos veces en la primera ejecución, para no perder acceso por una errata.
2. **Limpieza de memoria**: `sodium_memzero` borra la contraseña tras derivar la clave, y la clave al salir del programa.
3. **Formato v2**: la cabecera del archivo pasa a ser `v2|opslimit|memlimit|salt|nonce|cifrado`. Los parámetros de Argon2id viajan dentro del propio archivo, de modo que subir el coste de derivación en el futuro no rompe las bases de datos antiguas.
4. **Aviso** mediante `QMessageBox` cuando la contraseña maestra es incorrecta (sin retardo).
5. **Permisos 0600** en la base de datos (solo el propietario puede leerla/escribirla).
6. **Checks de error** en `guardar()`: aborta si el cifrado falla o el archivo no se abre, evitando pérdida de datos.
7. **Try-catch** en el parsing del formato v2, evitando crashes con archivos corruptos.
8. **Validación de nombres**: rechaza caracteres (`|`, `\n`, `\r`) que podrían corromper la base de datos.

### Fase 5 — Interfaz gráfica con Qt ✅
Migración de la consola a una ventana de escritorio con Qt Widgets:
- ✅ **Refactor "cerebro sin boca"**: las funciones de lógica ya no imprimen nada; devuelven datos (`string`/`bool`) y es quien llama quien decide cómo mostrarlos. Condición previa para poder conectar cualquier interfaz.
- ✅ **Proyecto CMake + primera ventana**: migrado a `CMakeLists.txt` con Qt6, clase `MainWindow` (hereda de `QMainWindow`) y bucle de eventos `QApplication::exec()`.
- ✅ **Layout completo**: formulario (app), botones de acción (generar, buscar, listar, eliminar), área de resultados y fila de utilidades (copiar, limpiar, modo seguro, salir).
- ✅ **Señales/slots conectados**: cada botón enlaza con el módulo lógico mediante lambdas; generar y eliminar re-cifran y guardan la base de datos al instante.
- ✅ **Tema oscuro**: paleta negra con ámbar/dorado extraída del logo, aplicada con hoja de estilos QSS; logo incrustado vía recursos Qt (`recursos.qrc`).
- ✅ **Modo seguro**: casilla que deshabilita el botón Generar para consultar sin riesgo de sobrescribir; «Limpiar» lo rehabilita.
- ✅ Botón «Copiar al portapapeles» conectado al portapapeles con auto-borrado a los 15s. Si la app no existe, muestra la lista de apps guardadas.
- ✅ Contraseña maestra introducida mediante diálogo gráfico personalizado con logo y separador, en vez de terminal.

### Fase 6 — Publicación ✅
Publicación del código en GitHub bajo licencia MIT, con este README y sin datos sensibles en el repositorio.

### Fase 7 — Host nativo (Native Messaging) ✅
Puente para que una extensión de Firefox se comunique con Gawain:
- **`gawain-host`**: segundo ejecutable que lee/escribe JSON por stdin/stdout (protocolo newline-delimited de Firefox Native Messaging).
- Opera sobre la **misma base de datos** que el GUI (`~/.gawain/archivo.txt`).
- Cada operación muestra el diálogo de contraseña maestra (misma seguridad que el GUI).
- Ops disponibles: `generate` (genera sin guardar), `save` (guarda en la BD), `get` (consulta), `ping`, `quit`.
- Manifest en `native-messaging-hosts/gawain-host.json` + script `scripts/instalar_host.sh`.

## Compilación

Requisitos: compilador C++17, CMake, Qt 6 (Widgets) y libsodium.

```bash
# Debian/Ubuntu
sudo apt install build-essential cmake qt6-base-dev libsodium-dev

cmake -B build
cmake --build build
./build/gawain          # GUI
./build/gawain-host     # Host nativo (Native Messaging)
```

## Instalar host para Firefox

```bash
bash scripts/instalar_host.sh
```

Copia el manifest a `~/.mozilla/native-messaging-hosts/` con la ruta absoluta del binario.

## Uso

1. Al arrancar se abre un diálogo personalizado (con logo) para la contraseña maestra; si es la primera vez se pide dos veces para confirmar.
2. Se abre la ventana: escribe el nombre de la app y usa **Generar**, **Buscar**, **Listar** o **Eliminar**.
3. Cada cambio (generar/eliminar) se re-cifra y guarda al instante en `~/.gawain/archivo.txt`.
4. «Modo seguro» bloquea Generar; «Limpiar» borra los campos y lo rehabilita.
5. En siguientes ejecuciones solo se pide la contraseña maestra; si es incorrecta aparece un aviso y se cierra la aplicación.

### Native Messaging (host)

El host recibe JSON por stdin y responde JSON por stdout:

| Op | Mensaje | Respuesta |
|---|---|---|
| `ping` | `{"op":"ping"}` | `{"ok":true}` |
| `generate` | `{"op":"generate","app":"github.com"}` | `{"ok":true,"password":"..."}` |
| `save` | `{"op":"save","app":"github.com","password":"..."}` | `{"ok":true}` |
| `get` | `{"op":"get","app":"github.com"}` | `{"ok":true,"password":"..."}` |
| `quit` | `{"op":"quit"}` | *(el host termina)* |

Cada op (excepto `ping`/`quit`) muestra el diálogo de maestra y opera sobre la BD.

### Atajos de teclado

| Atajo | Acción |
|---|---|
| `Ctrl+Shift+C` | Copiar contraseña al portapapeles |
| `Ctrl+F` | Buscar contraseña de la app |
| `Ctrl+L` | Listar todas las contraseñas |
| `Ctrl+D` | Eliminar entrada de la app |
| `Ctrl+S` | Activar/desactivar modo seguro |
| `Ctrl+X` | Limpiar campos y resultados |
| `Ctrl+Q` | Salir de la aplicación |

## Formato de la base de datos

```
v2|opslimit|memlimit|salt|nonce|ciphertext
   decimal |decimal| hex |hex |    hex
```

- `opslimit` / `memlimit`: coste de Argon2id usado al crear ese archivo.
- `salt`: 16 bytes que garantizan claves distintas para la misma contraseña.
- `nonce`: 12 bytes aleatorios por operación de cifrado.
- `ciphertext`: datos cifrados + tag de autenticación GCM (16 bytes).

## Estructura

```
main.cpp         arranque: desbloqueo + QApplication
main_window.*    ventana principal (Qt): layouts, tema oscuro y señales/slots
gawain.cpp/h     lógica: gestor de contraseñas + clase Vault
persistence.cpp  guardar/cargar la base de datos
cifrado.cpp/h    libsodium: Argon2id + AES-256-GCM, formato v2
host.cpp         host nativo (Native Messaging): despacho JSON + diálogo maestra
recursos.qrc     recursos embebidos (logo.png)
native-messaging-hosts/  manifest del host para Firefox
scripts/         scripts de instalación
```

## Licencia

MIT — ver [LICENSE](LICENSE).
