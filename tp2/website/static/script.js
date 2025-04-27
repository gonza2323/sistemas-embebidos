"use strict";


const socket = io();


const illuminationField = document.querySelector('#illumination-field');
const illuminationIndicator = document.querySelector('#illumination-value');
const readLuminosityCheckbox = document.querySelector("#read-luminosity");
const alarmIndicator = document.querySelector("#alarm");


socket.on("read_on", () => {
    readLuminosityCheckbox.checked = true;
    illuminationField.classList.remove("disabled");
});

socket.on("read_off", () => {
    readLuminosityCheckbox.checked = false;
    illuminationField.classList.add("disabled");
    alarmIndicator.classList.add("hidden");
});

socket.on("alarm", (data) => {
    alarmIndicator.classList.remove("hidden");
});

socket.on("illumination_update", (data) => {
    const illumination = data.illumination;
    illuminationIndicator.textContent = illumination + '%';
});

async function update(event) {
    try {
        const response = await fetch('update', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({
                readIllumination: readLuminosityCheckbox.checked
            })
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
    }

    if (readLuminosityCheckbox.checked) {
        illuminationField.classList.remove('disabled');
        illuminationIndicator.textContent = "Cargando..."
    } else {
        illuminationField.classList.add('disabled');
        alarmIndicator.classList.add("hidden");
    }
}

readLuminosityCheckbox.addEventListener("change", update);
