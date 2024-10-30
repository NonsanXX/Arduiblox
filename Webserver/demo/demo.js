function startConnect() {

    clientID = "clientID - " + parseInt(Math.random() * 100);

    host = "mqtt-dashboard.com";
    port = 8000;
    userId = document.getElementById("username").value;
    password = document.getElementById("password").value;

    document.getElementById("messages").innerHTML += "<span> Connecting to " + host + "on port " + port + "</span><br>";
    document.getElementById("messages").innerHTML += "<span> Using the client Id " + clientID + " </span><br>";

    client = new Paho.MQTT.Client(host, Number(port), clientID);

    client.onConnectionLost = onConnectionLost;
    client.onMessageArrived = onMessageArrived;

    client.connect({
        onSuccess: onConnect,
        onFailure: onFailure,
        userName: userId,
        password: password
    });


}

function onConnect() {

    console.log(`Connected to ${host}:${port} successfully.`)
    topic = document.getElementById("topic_s").value;
    document.getElementById("messages").innerHTML += "<span> Subscribing to topic " + topic + "</span><br>";
    client.subscribe(topic);
}

// Connection failure callback
function onFailure(message) {
    console.log("Connection failed: " + message.errorMessage);
    updateConnectionStatus(false);
    setTimeout(connect, 5000); // Try to reconnect after 5 seconds
}

function onConnectionLost(responseObject) {
    document.getElementById("messages").innerHTML += "<span> ERROR: Connection is lost.</span><br>";
    if (responseObject != 0) {
        document.getElementById("messages").innerHTML += "<span> ERROR:" + responseObject.errorMessage + "</span><br>";
    }
}

function onMessageArrived(message) {
    console.log("OnMessageArrived: " + message.payloadString);
    document.getElementById("messages").innerHTML += "<span> Topic:" + message.destinationName + "| Message : " + message.payloadString + "</span><br>";
}

function startDisconnect() {
    console.log("Disconnected from MQTT broker");
    client.disconnect();
    document.getElementById("messages").innerHTML += "<span> Disconnected. </span><br>";
}

function publishMessage() {
    msg = document.getElementById("Message").value;
    topic = document.getElementById("topic_p").value;

    Message = new Paho.MQTT.Message(msg);
    Message.destinationName = topic;

    client.send(Message);
    document.getElementById("messages").innerHTML += "<span> Message to topic " + topic + " is sent </span><br>";


}
