function encontrarFormulario() {
    const campoPassword = document.querySelector('input[type="password"]');
    if (!campoPassword) {
        return null;
    }else
    {
        return campoPassword.closest('form');
    }
}

function extraerContraseña(form) {
    const input = form.querySelector('input[type="password"]');
    if (input) {
        return input.value
    }
    else{
        return ""
    }
}

export default {encontrarFormulario, extraerContraseña}