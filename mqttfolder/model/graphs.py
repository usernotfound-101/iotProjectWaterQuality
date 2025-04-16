from pymongo import MongoClient
import matplotlib.pyplot as plt

client = MongoClient('localhost',27017)
db = client.flask_database
sensor_data_collection = db.mideval

