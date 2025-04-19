"use strict";


// audio
const UPDATE_INTERVAL = 100;
const SENSITIVITY = 5;
// iluminación
const CHART_DURATION = 60000;
const MAX_INTERRUPTION_DURATION = 7500;

let lastTimestamp;

Chart.register(ChartStreaming);
const socket = io();


navigator.mediaDevices.getUserMedia({ audio: true }).then(stream => {
    const scaleFactor = 2^SENSITIVITY
    const audioContext = new AudioContext();
    const source = audioContext.createMediaStreamSource(stream);
    const processor = audioContext.createScriptProcessor(2048, 1, 1);

    source.connect(processor);
    processor.connect(audioContext.destination);

    let lastEmit = Date.now();

    processor.onaudioprocess = (e) => {
        const now = Date.now();
        if (now - lastEmit < UPDATE_INTERVAL) {
            return;
        }
    
        const input = e.inputBuffer.getChannelData(0);
        let sum = 0;
        for (let i = 0; i < input.length; i++) {
            sum += input[i] * input[i];
        }
        const rms = Math.sqrt(sum / input.length);
        const volume = Math.log(1 + rms * scaleFactor) / Math.log(1 + scaleFactor);
    
        socket.emit('volume_data', volume);
        console.log(`Sent volume data: ${volume}`);
        lastEmit = now;
    };
});


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