import sys
import mysql.connector
import face_recognition
import cv2
import base64
import numpy as np
import configparser
import os

def load_db_config():
    config = configparser.ConfigParser()
    # Go one level up and into config directory
    config_path = os.path.join(os.path.dirname(__file__), '..', 'config', 'config.ini')
    
    if not os.path.exists(config_path):
        print("ERROR: config.ini not found")
        sys.exit(1)
    
    config.read(config_path)
    return {
        'host': config.get('Database', 'Host'),
        'user': config.get('Database', 'User'),
        'password': config.get('Database', 'Password'),
        'database': config.get('Database', 'DBName')
    }

def fetch_encodings_from_db():
    db_config = load_db_config()
    conn = mysql.connector.connect(**db_config)
    cursor = conn.cursor()
    cursor.execute("SELECT user_id, encoding FROM face_data")
    results = cursor.fetchall()
    cursor.close()
    conn.close()

    encodings = []
    for user_id, b64_encoding in results:
        arr = np.frombuffer(b64_encoding, dtype=np.float64)
        if arr.shape[0] == 128:
            encodings.append((user_id, arr))
    return encodings

def recognize_face(image_path):
    image = cv2.imread(image_path)
    if image is None:
        print("ERROR: Failed to load image")
        return

    rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    boxes = face_recognition.face_locations(rgb)
    if not boxes:
        print("ERROR: No face found")
        return

    new_encoding = face_recognition.face_encodings(rgb, boxes)[0]
    known_encodings = fetch_encodings_from_db()

    for user_id, known_encoding in known_encodings:
        match = face_recognition.compare_faces([known_encoding], new_encoding, tolerance=0.45)
        if match[0]:
            print(user_id)
            return

    print("ERROR: No match found")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("ERROR: No image path given")
    else:
        recognize_face(sys.argv[1])
