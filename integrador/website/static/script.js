"use strict";


// iluminación
const ARDUINO_ILLMUNATION_CHART_DURATION = 10000;
const MAX_INTERRUPTION_DURATION = 7500;
// audio
const UPDATE_INTERVAL = 100;
const SENSITIVITY = 3;
const LOCAL_VOLUME_CHART_DURATION = 10000;

let dataType = "volume";

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


const ctx2 = document.getElementById('arduino-buttons').getContext('2d');
const arduinoButtons = new Chart(ctx2, {
    type: 'line',
    data: {
        datasets: [{
            label: 'Botón 2',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgb(255, 33, 33)',
            borderWidth: 2,
            pointRadius: 0,
            data: []
        },{
            label: 'Botón 3',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgb(26, 41, 250)',
            borderWidth: 2,
            pointRadius: 0,
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
                    delay: 100,
                    refresh: 50,
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
                    text: 'Botones'
                },
                max: 1.2
            }
        },
        plugins: {
            title: {
                display: true,
                text: 'Actividad botones'
            },
            legend: {
                display: true
            }
        },
        animation: {
            duration: 100
        }
    }
});


const ctx3 = document.getElementById('local-volume').getContext('2d');
const localVolume = new Chart(ctx3, {
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


const ctx4 = document.getElementById('local-illumination').getContext('2d');
const localIllumination = new Chart(ctx4, {
    type: 'line',
    data: {
        datasets: [{
            label: 'Iluminación',
            backgroundColor: 'rgba(75, 192, 192, 0.2)',
            borderColor: 'rgb(25, 193, 34)',
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
                    text: 'Iluminación (%)'
                },
                max: 100
            }
        },
        plugins: {
            title: {
                display: true,
                text: 'Iluminación (local) en función del tiempo'
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


socket.on('connect', () => {
    console.log('Connected to server');
});


let lastIlluminationUpdate;
socket.on('illumination_update', function (data) {
    if (lastIlluminationUpdate && data.timestamp - lastIlluminationUpdate > MAX_INTERRUPTION_DURATION) {
        arduinoIllumination.data.datasets[0].data.push({ x: data.timestamp, y: null });
    }
    
    arduinoIllumination.data.datasets[0].data.push({
        x: data.timestamp,
        y: data.illumination
    });

    lastIlluminationUpdate = data.timestamp;
});


let lastButtonUpdate;
socket.on('buttons_update', function (data) {
    if (lastButtonUpdate && data.timestamp - lastButtonUpdate > MAX_INTERRUPTION_DURATION) {
        arduinoButtons.data.datasets[0].data.push({ x: data.timestamp, y: null });
        arduinoButtons.data.datasets[1].data.push({ x: data.timestamp, y: null });
    }

    arduinoButtons.data.datasets[data.button - 2].data.push({
        x: data.timestamp,
        y: data.buttonState ? 1 : 0
    });
    
    lastButtonUpdate = data.timestamp;
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
    
        if (dataType === "volume")
            socket.emit('data', volume);

        localVolume.data.datasets[0].data.push({
            x: now,
            y: volume * 100
        });
        
        lastEmit = now;
    };
});

try {
    const ilm = document.getElementById('ilm');
    const out = document.getElementById('out');
    const lightSensor = new AmbientLightSensor();
    setInterval(() => {
        const now = Date.now();
        const lightLevel = lightSensor.illuminance;
        const lightPercentage = lightLevel / 60000

        if (dataType === "illumination")
            socket.emit("data", lightPercentage);

        ilm.textContent = lightPercentage;
        localIllumination.data.datasets[0].data.push({
            x: now,
            y: lightPercentage * 100
        });
    }, 100);

    lightSensor.addEventListener('error', event => {
        console.error('Sensor error:', event.error.name, event.error.message);
        out.textContent = 'Sensor error:' + event.error.name, event.error.message;
    });

    lightSensor.start();
} catch (err) {
    console.error('Ambient Light Sensor not supported or blocked:', err);
    out.textContent = 'Ambient Light Sensor not supported or blocked:' + err
}
