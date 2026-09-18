import detector from './detector.js'

function autofill(datos) {
    if (!datos.dominio || datos.dominio !== window.location.hostname) {
        return false;
    }

    const form = detector.encontrarFormulario();
    if (!form) {
        return false;
    }

    const campoPassword = form.querySelector('input[type="password"]');
    if (!campoPassword || campoPassword.value !== '') {
        return false;
    }

    campoPassword.value = datos.contraseña;
    return true;
}

export default { autofill };
