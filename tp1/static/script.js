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

        const json = await response.json();
        console.log(responseText);

    } catch (error) {
        console.error('There was an error:', error);
    }
}

window.onload = function () {
    let form = document.querySelector('form');
    form.addEventListener('input', sendUpdate);
};
