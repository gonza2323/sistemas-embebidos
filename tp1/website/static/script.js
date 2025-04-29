"use strict";


const MIN_UPDATE_INTERVAL = 350;
let rangeUpdateInterval;

const socket = io();

const form = document.querySelector('form');
const illuminationIndicator = document.querySelector('#illumination');


socket.on("illumination_update", (data) => {
    const illumination = data.illumination;
    illuminationIndicator.textContent = illumination + '%';
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

        if (!response.ok) {
            const errorData = await response.json().catch(() => ({}));
            throw new Error(errorData.message || `HTTP error ${response.status}`);
        }

    } catch (error) {
        console.error('Request failed::', error);
    }
}

document.querySelectorAll("input[type='range']")
    .forEach(range => {
        range.addEventListener("mousedown", event => {
            rangeUpdateInterval = setInterval(sendUpdate, MIN_UPDATE_INTERVAL, event);
        })
        range.addEventListener("mouseup", event => {
            clearInterval(rangeUpdateInterval);
            setTimeout(() => sendUpdate(event), MIN_UPDATE_INTERVAL);
        })
    });

document.querySelector("#led13").addEventListener("input", sendUpdate);
