"use strict";


const CHART_DURATION = 60000;
const MAX_INTERRUPTION_DURATION = 7500;


let lastTimestamp;


Chart.register(ChartStreaming);

const socketHost = window.location.hostname === 'localhost' || window.location.hostname === '127.0.0.1'
    ? 'http://localhost:5000'
    : 'https://embebidos.ddns.net';

const socket = io(socketHost);


const ctx = document.getElementById('chart').getContext('2d');
const chart = new Chart(ctx, {
    type: 'line',
    data: {
        datasets: [{
            label: 'Iluminación',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgb(240, 51, 51)',
            borderWidth: 2,
            pointRadius: 2,
            data: []
        }]
    },
    options: {
        scales: {
            x: {
                title: {
                    display: true,
                    text: 'Hora (UTC-3)'
                },
                type: 'realtime',
                realtime: {
                    duration: CHART_DURATION,
                    refresh: 1000,
                    delay: 1000,
                    pause: false,
                    ttl: CHART_DURATION * 2
                },
                ticks: {
                    source: 'auto',
                    autoSkip: true
                }
            },
            y: {
                beginAtZero: true,
                title: {
                    display: true,
                    text: 'Iluminación (%)'
                },
                max: 100
            }
        },
        plugins: {
            title: {
                display: true,
                text: 'Iluminación en función del tiempo'
            },
            legend: {
                display: false
            }
        },
        animation: {
            duration: 100
        }
    }
});


socket.on('new_data_point', function (data) {
    if (lastTimestamp && data.timestamp - lastTimestamp > MAX_INTERRUPTION_DURATION)
        chart.data.datasets[0].data.push({ x: data.timestamp, y: null });
    
    chart.data.datasets[0].data.push({
        x: data.timestamp,
        y: data.illumination
    });

    lastTimestamp = data.timestamp;
});