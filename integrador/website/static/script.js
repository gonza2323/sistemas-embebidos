"use strict";


// iluminación
const ARDUINO_ILLMUNATION_CHART_DURATION = 10000;
const MAX_INTERRUPTION_DURATION = 7500;
// audio
const UPDATE_INTERVAL = 100;
const SENSITIVITY = 3;
const LOCAL_VOLUME_CHART_DURATION = 10000;

let lastTimestamp;

Chart.register(ChartStreaming);
const socket = io();


const ctx = document.getElementById('arduino-illumination').getContext('2d');
const arduinoIllumination = new Chart(ctx, {
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
                    duration: ARDUINO_ILLMUNATION_CHART_DURATION,
                    delay: 750,
                    refresh: 500,
                    pause: false,
                    ttl: ARDUINO_ILLMUNATION_CHART_DURATION * 2
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


const ctx2 = document.getElementById('local-volume').getContext('2d');
const localVolume = new Chart(ctx2, {
    type: 'line',
    data: {
        datasets: [{
            label: 'Volumen',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgb(51, 54, 240)',
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
                    duration: LOCAL_VOLUME_CHART_DURATION,
                    delay: 50,
                    refresh: 100,
                    pause: false,
                    ttl: LOCAL_VOLUME_CHART_DURATION * 1.5
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
                    text: 'Volumen (%)'
                },
                max: 100
            }
        },
        plugins: {
            title: {
                display: true,
                text: 'Volumen de audio (local) en función del tiempo'
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
        arduinoIllumination.data.datasets[0].data.push({ x: data.timestamp, y: null });
    
    arduinoIllumination.data.datasets[0].data.push({
        x: data.timestamp,
        y: data.illumination
    });

    lastTimestamp = data.timestamp;
});


navigator.mediaDevices.getUserMedia({ audio: true }).then(stream => {
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
        const volume = Math.log(1 + rms * 2**SENSITIVITY) / Math.log(1 + 2**SENSITIVITY);
    
        socket.emit('volume_data', volume);

        localVolume.data.datasets[0].data.push({
            x: now,
            y: volume * 100
        });
        
        lastEmit = now;
    };
});