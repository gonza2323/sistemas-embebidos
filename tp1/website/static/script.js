"use strict";


const socket = io("http://localhost:5000");

const form = document.querySelector('form');
const illuminationIndicator = document.querySelector('#illumination');


socket.on("illumination_update", (data) => {
    const illumination = data.illumination;
    illuminationIndicator.textContent = illumination;
});

async function sendUpdate(event) {
    event.preventDefault();

    if (event.target.tagName.toLowerCase() !== 'input')
        return;

    const formData = new FormData(form);

    try {
        const json = JSON.stringify(Object.fromEntries(formData.entries()));
        const response = await fetch('update', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: json
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
    }
}

function updateUI(data) {
    const led09range = document.querySelector('#led09');
    const led10range = document.querySelector('#led10');
    const led11range = document.querySelector('#led11');
    const led13checkbox = document.querySelector('#led13');

    led09range.value = data['led09'];
    led10range.value = data['led10'];
    led11range.value = data['led11'];
    led13checkbox.checked = data['led13'];
    illuminationIndicator.textContent = data['illumination'];
}

window.onload = function () {
    form.addEventListener('input', sendUpdate);
};
