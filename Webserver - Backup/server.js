const mqtt = require('mqtt');
const express = require('express');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');

const app = express();
const server = http.createServer(app);
const wss = new WebSocket.Server({ server });

// Store scores and MQTT connection status
let scores = [];
let mqttStatus = 'connecting'; // 'connecting', 'connected', 'error'

app.use(express.static(path.join(__dirname, 'public')));

const clientId = "node_" + Math.random().toString(16).substring(2, 10);
const host = "mqtt-dashboard.com";
const topic = "arduiblox/simongame";

const client = mqtt.connect(`mqtt://${host}`, {
    clientId: clientId,
    clean: true,
    username: '',
    password: '',
    reconnectPeriod: 5000,
    port: 1883
});

client.on('connect', () => {
    console.log(`Connected to MQTT broker at ${host}`);
    mqttStatus = 'connected';
    client.subscribe(topic, (err) => {
        if (err) {
            console.error('Subscribe error:', err);
            mqttStatus = 'error';
        } else {
            console.log('Subscribed to:', topic);
        }
        broadcastMqttStatus();
    });
});

client.on('reconnect', () => {
    console.log('Attempting to reconnect to MQTT broker...');
    mqttStatus = 'connecting';
    broadcastMqttStatus();
});

client.on('error', (error) => {
    console.log("Connection error:", error);
    mqttStatus = 'error';
    broadcastMqttStatus();
});

client.on('offline', () => {
    console.log('MQTT client is offline');
    mqttStatus = 'error';
    broadcastMqttStatus();
});

client.on('message', (topic, message) => {
    try {
        const messageStr = message.toString();
        console.log('Received message:', messageStr);

        const payload = JSON.parse(messageStr);

        if (payload && typeof payload === 'object') {
            if (payload.score !== undefined && payload.userid !== undefined) {
                const newScore = {
                    player: payload.userid,
                    score: payload.score,
                    date: new Date().toISOString()
                };

                scores.push(newScore);
                scores.sort((a, b) => b.score - a.score);
                scores = scores.slice(0, 10);

                broadcastScores();
            }
        }
    } catch (e) {
        console.log("Message processing error:", e.message);
        console.log("Raw message:", message.toString());
    }
});

wss.on('connection', (ws) => {
    console.log('New WebSocket client connected');

    // Send both MQTT status and scores to new client
    ws.send(JSON.stringify({
        type: 'mqttStatus',
        status: mqttStatus
    }));

    // Always send scores regardless of MQTT status
    ws.send(JSON.stringify({
        type: 'scores',
        data: scores
    }));

    ws.on('close', () => {
        console.log('Client disconnected');
    });

    ws.on('error', (error) => {
        console.log('WebSocket error:', error);
    });
});

function broadcastMqttStatus() {
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            try {
                client.send(JSON.stringify({
                    type: 'mqttStatus',
                    status: mqttStatus
                }));
            } catch (e) {
                console.error('Broadcast error:', e);
            }
        }
    });
}

function broadcastScores() {
    wss.clients.forEach((client) => {
        if (client.readyState === WebSocket.OPEN) {
            try {
                client.send(JSON.stringify({
                    type: 'scores',
                    data: scores
                }));
            } catch (e) {
                console.error('Broadcast error:', e);
            }
        }
    });
}

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});

process.on('SIGINT', () => {
    console.log('Closing MQTT connection...');
    client.end(true, () => {
        console.log('MQTT connection closed');
        process.exit(0);
    });
});
