"use strict";


async function sendUpdate(event) {
    event.preventDefault();

    if (event.target.tagName.toLowerCase() !== 'input')
        return;

    const formData = new FormData(document.querySelector('form'));

    try {
        const response = await fetch('update', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(Object.fromEntries(formData.entries()))
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
    updateLuminosity(data['luminosity']);
}

function updateLuminosity(luminosity) {
    const ldrSpan = document.querySelector('#luminosity');
    ldrSpan.textContent = luminosity;
}

window.onload = function () {
    let form = document.querySelector('form');
    form.addEventListener('input', sendUpdate);
};
