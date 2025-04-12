"use strict";


const socketHost = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'
    ? 'http://localhost:5000'
    : 'https://embebidos.ddns.net';

const socket = io(socketHost);

let events = [];


socket.on("single_event", (event) => {
    events.push(event);
    renderTable();
});

socket.on("all_events", (receivedEvents) => {
    events = receivedEvents;
    renderTable();
})

async function updateTime(event) {
    try {
        const response = await fetch('sync-time', {
            method: 'POST',
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
    }
}

async function requestEvents(event) {
    try {
        const response = await fetch('request-events', {
            method: 'GET',
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
    }
}

async function eraseMemory(event) {
    try {
        const response = await fetch('erase-memory', {
            method: 'POST',
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

        events = [];
        renderTable();

    } catch (error) {
        console.error('There was an error:', error);
    }
}

function renderTable() {
    const noEventsPlaceholder = document.querySelector('#no-events-placeholder');
    const tbody = document.querySelector('tbody');
    tbody.innerHTML = '';

    if (events.length === 0) {
        noEventsPlaceholder.hidden = false;
        return;
    }

    const fragment = document.createDocumentFragment();
    
    events.forEach(e => {
        const row = document.createElement('tr');
        
        const event = row.insertCell();
        const tiemstamp = row.insertCell();
        const time = row.insertCell();

        event.textContent = e.event;
        tiemstamp.textContent = e.timestamp;
        time.textContent = e.time;

        fragment.prepend(row);
    });

    noEventsPlaceholder.hidden = true;
    tbody.appendChild(fragment);
}

document.querySelector('#update-time').addEventListener('click', updateTime);
document.querySelector('#request-events').addEventListener('click', requestEvents);
document.querySelector('#erase-memory').addEventListener('click', eraseMemory);

requestEvents();
renderTable();
