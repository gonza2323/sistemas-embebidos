"use strict";


Chart.register(ChartStreaming);

const ARDUINO_ILLMUNATION_CHART_DURATION = 10000;
const MAX_INTERRUPTION_DURATION = 7500;
const LOCAL_VOLUME_CHART_DURATION = 10000;


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
        responsive: true,
        maintainAspectRatio: false,
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
                text: 'Iluminación del Arduino en función del tiempo'
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
        responsive: true,
        maintainAspectRatio: false,
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
                text: 'Estado de los botones del Arduino en función del tiempo'
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
        responsive: true,
        maintainAspectRatio: false,
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
        responsive: true,
        maintainAspectRatio: false,
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
