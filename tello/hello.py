import tello

drone = tello.Tello('192.168.10.1',8888)
drone.takeoff() 
time.sleep(5)