import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
import joblib
from bson.json_util import dumps
import json
from pymongo import MongoClient

client = MongoClient('localhost', 27017)
db = client.flask_database
sensor_data = db.mideval

raw_data = list(sensor_data.find({}, {"_id": 0}))
data = pd.DataFrame(raw_data)

def label_maintenance(row):
    return int(row['PH'] < 6.5 or row['PH'] > 8.5 or row['TDS'] > 500 or row['TURBIDITY'] > 5)

data['label']=data.apply(label_maintenance, axis=1)

x = data[["PH","TURBIDITY","TDS"]]
y = data["label"]

X_train, X_test, y_train, y_test = train_test_split(x,y, test_size=0.2, random_state=42)

model = RandomForestClassifier()
model.fit(X_train, y_train)

print(classification_report(y_test, model.predict(X_test)))

joblib.dump(model, 'water_quality_model.pkl')