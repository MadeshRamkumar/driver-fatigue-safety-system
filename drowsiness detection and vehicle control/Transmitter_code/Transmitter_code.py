import cv2
import winsound  
import requests  
import threading  

ESP8266_IP = "Replace with your ESP8266's IP address"  


face_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_frontalface_default.xml')
eye_cascade = cv2.CascadeClassifier(cv2.data.haarcascades + 'haarcascade_eye.xml')

EYE_CLOSED_FRAMES = 20 
eye_closed_counter = 0
last_state = "NOT_DROWSY"  

def send_http_request(state):
    try:
        response = requests.get(f"{ESP8266_IP}/control?state={state}")
        if response.status_code == 200:
            print(f"Signal sent to ESP8266: {state}")
        else:
            print(f"Failed to send signal to ESP8266, status code: {response.status_code}")
    except Exception as e:
        print(f"Error connecting to ESP8266: {e}")

# Start video capture
cap = cv2.VideoCapture(0)

while True:
    ret, frame = cap.read()
    if not ret:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)

    faces = face_cascade.detectMultiScale(gray, scaleFactor=1.1, minNeighbors=5, minSize=(30, 30))

    drowsy_state = "NOT_DROWSY"  

    for (x, y, w, h) in faces:
        cv2.rectangle(frame, (x, y), (x+w, y+h), (255, 0, 0), 2)
        roi_gray = gray[y:y+h, x:x+w]
        roi_color = frame[y:y+h, x:x+w]

        eyes = eye_cascade.detectMultiScale(roi_gray, scaleFactor=1.1, minNeighbors=10, minSize=(15, 15))

        if len(eyes) == 0:
            eye_closed_counter += 1
        else:
            eye_closed_counter = 0  


        if eye_closed_counter >= EYE_CLOSED_FRAMES:
            drowsy_state = "DROWSY"
            cv2.putText(frame, "DROWSINESS ALERT!", (10, 50),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 0, 255), 3)
            winsound.Beep(1000, 500)  


    cv2.putText(frame, f"State: {drowsy_state}", (10, 100),
                cv2.FONT_HERSHEY_SIMPLEX, 1, (0, 255, 0) if drowsy_state == "NOT_DROWSY" else (0, 0, 255), 2)

    if drowsy_state != last_state:
        last_state = drowsy_state
        threading.Thread(target=send_http_request, args=(drowsy_state,)).start()

    cv2.imshow("Drowsiness Detection", frame)

    # Exit on pressing 'q'
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
