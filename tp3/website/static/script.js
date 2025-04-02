"use strict";


const socket = io("http://localhost:5000");

socket.on("hello", () => {
    console.log('hello')
});

async function updateTime(event) {
    try {
        const response = await fetch('update-time', {
            method: 'POST',
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

    } catch (error) {
        console.error('There was an error:', error);
    }
}

async function getEvents(event) {
    try {
        const response = await fetch('get-events', {
            method: 'GET',
        });

        if (!response.ok)
            throw new Error('Network response was not ok ' + response.statusText);

        setTableData();

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

        emptyTable();

    } catch (error) {
        console.error('There was an error:', error);
    }
}

function emptyTable() {

}

function setTableData() {

}

document.querySelector('#update-time').addEventListener('click', updateTime);
document.querySelector('#get-events').addEventListener('click', getEvents);
document.querySelector('#erase-memory').addEventListener('click', eraseMemory);
