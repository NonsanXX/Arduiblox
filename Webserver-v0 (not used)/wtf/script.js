// Store scores in memory
let scores = [];

// MQTT Configuration
const clientId = "web_" + Math.random().toString(16).substring(2, 10);
const host = "mqtt-dashboard.com";
const topic = "arduiblox/simongame";

// Create MQTT client
const client = mqtt.connect('mqtt://mqtt-dashboard.com', {
    clientId: clientId,
    clean: true,
    reconnectPeriod: 5000, // Try to reconnect every 5 seconds
    port: 8000,
    protocol: 'ws', // Explicitly set WebSocket protocol
});

// Connect handler
client.on('connect', () => {
    console.log("Connected to MQTT on " + host);
    updateConnectionStatus(true);
    client.subscribe(topic);
    client.publish(topic, JSON.stringify({ type: 'score' }));
});

// Error handler
client.on('error', (error) => {
    console.log("Connection error:", error);
    updateConnectionStatus(false);
});

// Connection closed handler
client.on('close', () => {
    console.log("Connection closed");
    updateConnectionStatus(false);
});

// Message handler
client.on('message', (topic, message) => {
    try {
        const payload = JSON.parse(message.toString());
        if (payload.score !== undefined && payload.userid !== undefined) {
            const newScore = {
                player: payload.userid,
                score: payload.score,
                date: new Date().toISOString()
            };

            // Add new score and sort
            scores.push(newScore);
            scores.sort((a, b) => b.score - a.score);

            // Keep only top 10 scores
            scores = scores.slice(0, 10);

            // Update display
            updateScoreTable();
        }
    } catch (e) {
        console.error("Error processing message:", e);
    }
});

// Update connection status display
function updateConnectionStatus(connected) {
    const statusDiv = document.getElementById('connectionStatus');
    if (connected) {
        statusDiv.className = 'connection-status connected';
        statusDiv.textContent = 'Connected to MQTT';
    } else {
        statusDiv.className = 'connection-status disconnected';
        statusDiv.textContent = 'Disconnected from MQTT';
    }
}

// Update score table
function updateScoreTable() {
    const tableBody = document.getElementById('scoreTableBody');
    tableBody.innerHTML = '';

    scores.forEach((score, index) => {
        const row = document.createElement('tr');
        row.innerHTML = `
            <td>${index + 1}</td>
            <td>${score.player}</td>
            <td>${score.score}</td>
            <td>${new Date(score.date).toLocaleString()}</td>
        `;
        tableBody.appendChild(row);
    });
}