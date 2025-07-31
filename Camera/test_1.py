import cv2

def open_scaled_camera(camera_id=1, scale=0.5):
    cap = cv2.VideoCapture(camera_id)

    # Request high resolution to maximize field of view
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

    if not cap.isOpened():
        print("Cannot open camera")
        return

    print("Camera opened successfully at resolution:",
          int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), "x",
          int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)))

    while True:
        ret, frame = cap.read()
        if not ret:
            print("Can't receive frame (stream end?). Exiting ...")
            break

        # Resize frame for display
        frame_resized = cv2.resize(frame, None, fx=scale, fy=scale)

        cv2.imshow('Wide Angle Camera (Scaled)', frame_resized)

        # Exit on ESC
        if cv2.waitKey(1) == 27:
            break

    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    # Use scale=0.5 to fit high-res video into screen
    open_scaled_camera(scale=0.5)
