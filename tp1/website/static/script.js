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

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
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
