import matplotlib.pyplot as plt
import io
import base64
from flask import Blueprint, jsonify
from pymongo import MongoClient

# Initialize Blueprint
graphs_bp = Blueprint("graphs", __name__)

# MongoDB connection
client = MongoClient('localhost', 27017)
db = client.flask_database
sensor_data_collection = db.mideval

VALID_KEYS = {"TDS", "TURBIDITY", "PH"}

def generate_graph(sensor_type):
    # Fetch data from MongoDB
    data = list(sensor_data_collection.find({}, {"_id": 0, sensor_type: 1, "timestamp": 1}).sort("timestamp", 1))
    if not data:
        return None

    # Extract timestamps and sensor values
    timestamps = [entry["timestamp"] for entry in data]
    values = [entry[sensor_type] for entry in data]

    # Create the plot
    plt.figure(figsize=(10, 6))
    plt.plot(timestamps, values, marker="o", label=sensor_type)
    plt.xlabel("Timestamp")
    plt.ylabel(sensor_type)
    plt.title(f"{sensor_type} Readings Over Time")
    plt.legend()
    plt.grid(True)

    # Save the plot to a BytesIO object
    img = io.BytesIO()
    plt.savefig(img, format="png")
    img.seek(0)
    plt.close()

    # Encode the image as base64
    graph_url = base64.b64encode(img.getvalue()).decode()
    return graph_url

@graphs_bp.route("/graphs/<sensor_type>")
def render_graph(sensor_type):
    if sensor_type not in VALID_KEYS:
        return jsonify({"error": "Invalid sensor type"}), 400

    graph_url = generate_graph(sensor_type)
    if not graph_url:
        return jsonify({"error": "No data available"}), 404

    return f'<img src="data:image/png;base64,{graph_url}" alt="{sensor_type} graph">'