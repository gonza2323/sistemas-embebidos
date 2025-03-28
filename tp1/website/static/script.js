"use strict";


const socket = io("http://localhost:5000");

const form = document.querySelector('form');
const illuminationIndicator = document.querySelector('#illumination');


socket.on("illumination_update", (data) => {
    const illumination = data.illumination;
    illuminationIndicator.textContent = illumination + ' lx';
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

window.onload = function () {
    form.addEventListener('input', sendUpdate);
};
