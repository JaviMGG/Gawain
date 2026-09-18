import detector from './detector.js';
import autofill from './autofill.js';

const listaCallbacks = [];
let listenerRegistrado = false;

function manejadorSubmit(evento) {
    const form = evento.target.closest('form');
    if (!form) {
        return;
    }
    const contraseña = detector.extraerContraseña(form);
    const dominio = window.location.hostname;
    for (const callback of listaCallbacks) {
        callback({ dominio, contraseña });
    }
}

function registroUnicoListener() {
    if (!listenerRegistrado) {
        window.addEventListener('submit', manejadorSubmit);
        listenerRegistrado = true;
    }
}

function eliminarCallback(callback) {
    const indice = listaCallbacks.indexOf(callback);
    if (indice !== -1) {
        listaCallbacks.splice(indice, 1);
    }
}

function onLogin(callback) {
    if (!listaCallbacks.includes(callback)) {
        listaCallbacks.push(callback);
    }
    registroUnicoListener();
    return () => eliminarCallback(callback);
}

function detect(dominio) {
    let form = detector.encontrarFormulario();
    if (!form) {
        return null;
    }
    let contraseña = detector.extraerContraseña(form);
    if (contraseña == "") {
        return null;
    }
    return {dominio, contraseña}
}

export const Yvain = {detect, autofill: autofill.autofill, onLogin};