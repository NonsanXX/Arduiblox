const express = require('express');
const http = require('http');
const path = require('path');
const mqtt = require('mqtt');

const app = express();

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

let scores = [];

client.on('connect', () => {
    console.log(`Connected to MQTT broker at ${host}`);
    client.subscribe(topic, (err) => {
        if (err) {
            console.error('Subscribe error:', err);
        } else {
            console.log('Subscribed to:', topic);
        }
    });
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
            }
        }
    } catch (e) {
        console.log("Message processing error:", e.message);
        console.log("Raw message:", message.toString());
    }
});

app.get('/scores', (req, res) => {
    res.json(scores);
});

//app.use(express.static(path.join(__dirname, 'public')));

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => {
    console.log(`Server running at http://localhost:${PORT}`);
});

process.on('SIGINT', () => {
    console.log('Closing MQTT connection...');
    client.end(true, () => {
        console.log('MQTT connection closed');
        process.exit(0);
    });
});