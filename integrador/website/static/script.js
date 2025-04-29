"use strict";


// constantes
const SEND_UPDATE_INTERVAL = 100;
const DEFAULT_VOLUME_SENSITIVITY = 2;
const DEFAULT_LIGHT_SENSITIVITY = 2;


// socket
const socket = io();


// módulo de streaming de volumen de audio
const volumeStream = function() {
    let audioContext = null;
    let stream = null;
    let source = null;
    let processor = null;
    let active = false;
    let permissionGranted = false;
    let lastUpdateTime = null;
    let initInProgress = false;
    let volumeSensitivity = DEFAULT_VOLUME_SENSITIVITY;

    async function initialize() {
        if (permissionGranted || initInProgress) {
            return;
        }
        
        initInProgress = true;
        
        try {
            stream = await navigator.mediaDevices.getUserMedia({ audio: true });
            audioContext = new AudioContext();
            permissionGranted = true;

            source = audioContext.createMediaStreamSource(stream);
            processor = audioContext.createScriptProcessor(2048, 1, 1);
        
            source.connect(processor);
            processor.connect(audioContext.destination);
        
            processor.onaudioprocess = processAudio;
        } catch (error) {
            console.error('Audio permission denied or error:', error);
            permissionGranted = false;
        }

        initInProgress = false;
    }

    function processAudio(event) {
        if (lastUpdateTime === null) {
            lastUpdateTime = Date.now();
        }

        const now = Date.now();
        if (now - lastUpdateTime < SEND_UPDATE_INTERVAL) {
            return;
        }
    
        const input = event.inputBuffer.getChannelData(0);
        let sum = 0;
        for (let i = 0; i < input.length; i++) {
            sum += input[i] * input[i];
        }
        const rms = Math.sqrt(sum / input.length);
        const volume = rms ** (1 / volumeSensitivity)
    
        if (active) {
            socket.emit('data', volume);
        }

        localVolume.data.datasets[0].data.push({
            x: now,
            y: volume * 100
        });
        
        lastUpdateTime = now;
    }

    function setSensitivity(sensitivity) {
        volumeSensitivity = sensitivity;
    }

    function startStream() { active = true; }
    function stopStream() { active = false; }
    function isInitialized() { return permissionGranted; }

    
    return {
        initialize,
        startStream,
        stopStream,
        setSensitivity,
        isInitialized
     };
}();


// modulo de streaming de iluminación
const illuminationStream = function() {
    let lightSensor = null;
    let lightSensitivity = DEFAULT_LIGHT_SENSITIVITY;
    let active = false;
    let permissionGranted = false;
    let initInProgress = false;
    let maxIllumination = 60000;


    function initialize() {
        if (permissionGranted || initInProgress) {
            return;
        }
        initInProgress = true;
        const errorIndicator = document.querySelector('.error');

        try {
            
            lightSensor = new AmbientLightSensor();
            lightSensor.addEventListener('error', event => {
                console.error('Sensor error:', event.error.name, event.error.message);
            });

            lightSensor.start();
            permissionGranted = true;

            setInterval(processIllumination, SEND_UPDATE_INTERVAL)
        } catch (err) {
            console.error('Ambient Light Sensor not supported or blocked:', err);
        }
        initInProgress = false;
    }

    function processIllumination() {
        const now = Date.now();
        const lightLevel = lightSensor.illuminance;

        if (lightLevel > maxIllumination) {
            maxIllumination = lightLevel;
        }

        const lightPercentage = (lightLevel / maxIllumination) ** (1 / lightSensitivity);

        if (active) {
            socket.emit("data", lightPercentage);
        }

        localIllumination.data.datasets[0].data.push({
            x: now,
            y: lightPercentage * 100
        });
    }

    function setSensitivity(sensitivity) {
        lightSensitivity = sensitivity;
    }

    function startStream() { active = true; }
    function stopStream() { active = false; }
    function isInitialized() { return permissionGranted; }

    
    initialize();
    
    return {
        initialize,
        startStream,
        stopStream,
        setSensitivity,
        isInitialized
    };
}();


// aplicación
const app = function() {
    let streamingMode = null;

    async function setStreamingMode(mode) {
        if (mode === "volume") {
            if (!volumeStream.isInitialized()) {
                await volumeStream.initialize();
            }

            if (volumeStream.isInitialized()) {
                illuminationStream.stopStream();
                volumeStream.startStream();
                streamingMode = mode;
            }

        } else if (mode === "illumination") {
            if (!illuminationStream.isInitialized()) {
                illuminationStream.initialize();
            }

            if (illuminationStream.isInitialized()) {
                volumeStream.stopStream();
                illuminationStream.startStream();
                streamingMode = mode;
            }
        }

        ui.setStreamingMode(streamingMode);
    }

    setStreamingMode("volume");

    return { setStreamingMode };
}();


// interfaz gráfica
const ui = function() {
    function initialize() {
        document.querySelectorAll('input[name="streaming-mode"]').forEach( button =>
            button.addEventListener('change', event =>{
                const choice = event.target.value;
                app.setStreamingMode(choice);
            })
        )

        const volumeSensitivityRange = document.querySelector('#volume-sensitivity-range');
        const volumeSensitivityIndicator = document.getElementById('volume-sensitivity');
        volumeSensitivityRange.value = DEFAULT_VOLUME_SENSITIVITY;
        volumeSensitivityIndicator.textContent = (DEFAULT_VOLUME_SENSITIVITY * 100).toFixed(0) + '%';
        
        volumeSensitivityRange.addEventListener('input', (event) => {
            const sensitivity = Number(event.target.value);
            volumeStream.setSensitivity(sensitivity);
            volumeSensitivityIndicator.textContent = (sensitivity * 100).toFixed(0) + '%';
        })
        
        const lightSensitivityRange = document.querySelector('#light-sensitivity-range');
        const lightSensitivityIndicator = document.getElementById('light-sensitivity');
        lightSensitivityRange.value = DEFAULT_LIGHT_SENSITIVITY;
        lightSensitivityIndicator.textContent = (DEFAULT_LIGHT_SENSITIVITY * 100).toFixed(0) + '%';
        
        lightSensitivityRange.addEventListener('input', (event) => {
            const sensitivity = Number(event.target.value);
            illuminationStream.setSensitivity(sensitivity);
            lightSensitivityIndicator.textContent = (sensitivity * 100).toFixed(0) + '%';
        })
    }

    function setStreamingMode(mode) {
        const radios = document.querySelectorAll(`input[name="streaming-mode"]`);
        radios.forEach(radio => {
            radio.checked = radio.value === mode;
        });
    }

    initialize();

    return { setStreamingMode };
}()


// updates del servidor

let lastIlluminationUpdate;
socket.on('illumination_update', function (data) {
    const now = Date.now();

    if (lastIlluminationUpdate && now - lastIlluminationUpdate > MAX_INTERRUPTION_DURATION) {
        arduinoIllumination.data.datasets[0].data.push({ x: now, y: null });
    }
    
    arduinoIllumination.data.datasets[0].data.push({
        x: now,
        y: data.illumination
    });

    lastIlluminationUpdate = now;
});


let lastButtonUpdate;
socket.on('buttons_update', function (data) {
    const now = Date.now();

    if (lastButtonUpdate && now - lastButtonUpdate > MAX_INTERRUPTION_DURATION) {
        arduinoButtons.data.datasets[0].data.push({ x: now, y: null });
        arduinoButtons.data.datasets[1].data.push({ x: now, y: null });
    }

    arduinoButtons.data.datasets[data.button - 2].data.push({
        x: now,
        y: data.buttonState ? 1 : 0
    });
    
    lastButtonUpdate = now;
});
