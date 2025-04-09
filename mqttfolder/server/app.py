from flask import Flask, render_template, request, jsonify
import numpy as np
from pymongo import MongoClient
import paho.mqtt.client as mqtt
import threading
import json

app = Flask(__name__)
client = MongoClient('localhost',27017)
db = client.flask_database
sensor_data_collection = db.mideval

VALID_KEYS={"TDS","TURBIDITY","PH"}
MQTT_BROKER = "localhost"
MQTT_PORT = 1883
MQTT_TOPIC = "quality"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    print(f"Message recieved: {msg.topic} -> {msg.payload.decode()}")

    try:
        data = json.loads(msg.payload.decode())
        if isinstance(data, dict):
            if all(key in data for key in VALID_KEYS):
                filtered_data = {key: data[key] for key in VALID_KEYS}
                sensor_data_collection.insert_one(filtered_data)    
                print("Data inserted into MongoDB:", filtered_data)
            else:
                print("Invalid keys in data")
        else:
            print("Invalid data format")
    except Exception as e:
        print("Error parsing data:", e)

def start_mqtt():
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
    mqtt_client.loop_forever()

    #Flask routes

@app.route("/", methods=["GET"])
def index():
    recent_post = sensor_data_collection.find_one(sort=[("_id", -1)])
    return render_template("index.html")

@app.route("/data", methods=["GET"])
def get_data():
    data = list(sensor_data_collection.find({}, {"_id": 0}))
    return jsonify({"mideval":data})

@app.route("/test_db", methods=["GET"])
def test_db():
    try:
        db.admin.command('ping')
        return jsonify({"status":"MongoDB is running"})
    except Exception as e:
        return jsonify({"status":"MongoDB is not running", "error": str(e)})

@app.route("/insert", methods=["POST"])
def insert_data():
    data = request.json
    if not isinstance(data, dict):
        return jsonify({"error": "Invalid data format"}), 400
    if all(key in data for key in VALID_KEYS):
        sensor_data_collection.insert_one(data)
        return jsonify({"status": "Data inserted successfully"})
    else:
        return jsonify({"error": "Invalid keys in data"}), 400

if __name__=='__main__':
    mqtt_thread = threading.Thread(target=start_mqtt,daemon=True)
    mqtt_thread.start()
    app.run(host='0.0.0.0', port=5002, debug=True) 

