# face_register.py
import sys
import cv2
import face_recognition
import base64
import numpy as np

def main():
    if len(sys.argv) < 2:
        print("ERROR: No image path given")
        return

    image_path = sys.argv[1]
    image = cv2.imread(image_path)

    if image is None:
        print("ERROR: Failed to read image")
        return

    rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)
    faces = face_recognition.face_locations(rgb)

    if not faces:
        print("ERROR: No face detected")
        return

    encodings = face_recognition.face_encodings(rgb, faces)
    encoding_array = encodings[0]  # first face only

    # Convert to base64 string for safe transfer
    b64_encoded = "SUCCESS:" + base64.b64encode(encoding_array.tobytes()).decode('utf-8')
    print(b64_encoded)

if __name__ == "__main__":
    main()
