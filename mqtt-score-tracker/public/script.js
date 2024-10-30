const ws = new WebSocket(`ws://${window.location.host}`);

ws.onopen = () => {
    console.log('WebSocket connected');
};

ws.onclose = () => {
    console.log('WebSocket disconnected');
    setTimeout(() => {
        window.location.reload();
    }, 5000);
};

ws.onmessage = (event) => {
    const message = JSON.parse(event.data);
    
    if (message.type === 'mqttStatus') {
        updateConnectionStatus(message.status);
    } else if (message.type === 'scores') {
        updateScoreTable(message.data);
    }
};

function updateConnectionStatus(status) {
    const statusDiv = document.getElementById('connectionStatus');
    let statusText, statusClass, statusIcon;
    
    switch (status) {
        case 'connecting':
            statusText = 'Connecting to MQTT...';
            statusClass = 'status-bar connecting';
            statusIcon = `
                <svg class="w-5 h-5 mr-2 animate-spin" fill="none" viewBox="0 0 24 24">
                    <circle class="opacity-25" cx="12" cy="12" r="10" stroke="currentColor" stroke-width="4"></circle>
                    <path class="opacity-75" fill="currentColor" d="M4 12a8 8 0 018-8V0C5.373 0 0 5.373 0 12h4zm2 5.291A7.962 7.962 0 014 12H0c0 3.042 1.135 5.824 3 7.938l3-2.647z"></path>
                </svg>`;
            break;
        case 'connected':
            statusText = 'Connected to MQTT';
            statusClass = 'status-bar';
            statusIcon = `
                <svg class="w-5 h-5 mr-2" fill="currentColor" viewBox="0 0 20 20">
                    <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zm3.707-9.293a1 1 0 00-1.414-1.414L9 10.586 7.707 9.293a1 1 0 00-1.414 1.414l2 2a1 1 0 001.414 0l4-4z" clip-rule="evenodd"/>
                </svg>`;
            break;
        case 'error':
            statusText = 'MQTT Connection Error - Retrying in 5s...';
            statusClass = 'status-bar error';
            statusIcon = `
                <svg class="w-5 h-5 mr-2" fill="currentColor" viewBox="0 0 20 20">
                    <path fill-rule="evenodd" d="M10 18a8 8 0 100-16 8 8 0 000 16zM8.707 7.293a1 1 0 00-1.414 1.414L8.586 10l-1.293 1.293a1 1 0 101.414 1.414L10 11.414l1.293 1.293a1 1 0 001.414-1.414L11.414 10l1.293-1.293a1 1 0 00-1.414-1.414L10 8.586 8.707 7.293z" clip-rule="evenodd"/>
                </svg>`;
            break;
    }
    
    statusDiv.className = statusClass;
    statusDiv.innerHTML = `${statusIcon}${statusText}`;
}

function getTrophyEmoji(rank) {
    if (rank === 1) return '🏆';
    if (rank === 2) return '🥈';
    if (rank === 3) return '🥉';
    return rank;
}

function updateScoreTable(scores) {
    const tableBody = document.getElementById('scoreTableBody');
    tableBody.innerHTML = '';

    scores.forEach((score, index) => {
        const row = document.createElement('div');
        row.className = 'score-row';
        row.innerHTML = `
            <div class="rank-cell">
                <span class="rank-trophy">${getTrophyEmoji(index + 1)}</span>
            </div>
            <div class="player-cell">
                <div class="player-avatar">
                    ${score.player.charAt(0).toUpperCase()}
                </div>
                <div class="player-name">${score.player}</div>
            </div>
            <div>
                <span class="score-badge">${score.score}</span>
            </div>
            <div class="date-cell">
                ${new Date(score.date).toLocaleString()}
            </div>
        `;
        tableBody.appendChild(row);
    });
}