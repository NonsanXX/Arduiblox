const express = require('express');
const bodyParser = require('body-parser');
const path = require('path');
const app = express();
const port = 3000;

// Store scores in memory (you can replace this with a database later)
let highScores = [];

// Middleware
app.use(bodyParser.json());
app.use(express.static(path.join(__dirname, 'public')));

// Serve index.html for root route
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'public', 'index.html'));
});

// API endpoint to receive scores
app.post('/api/scores', (req, res) => {
    console.log('Received score:', req.body); // Debug log

    const { player, score, date } = req.body;

    highScores.push({
        player,
        score,
        date: date || new Date().toISOString()
    });

    // Sort scores in descending order
    highScores.sort((a, b) => b.score - a.score);

    // Keep only top 10 scores
    if (highScores.length > 10) {
        highScores = highScores.slice(0, 10);
    }

    res.json({ success: true, message: 'Score recorded successfully' });
});

// API endpoint to get scores
app.get('/api/scores', (req, res) => {
    res.json(highScores);
});

app.listen(port, '0.0.0.0', () => {
    console.log(`Server running at http://localhost:${port}`);
    console.log(`Serving static files from: ${path.join(__dirname, 'public')}`);
});