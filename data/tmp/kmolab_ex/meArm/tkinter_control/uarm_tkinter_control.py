'''
Original Author: Marks Calderon Niquin
Email: markscalderonniquin@gmail.com
github: hypagetech
Program modified to control uarm robot on CoppeliaSim through sliders GUI with TKinter
'''
from tkinter import *
from tkinter import messagebox
import sim
import time
import math

root = Tk()

class GUI(Frame):
    scales_l = []
    labels_l = []
    handles = []
    boolConnect = False
    clientID = -1
    base = 'joint'
    final = 'tip'

    def __init__(self):
        super().__init__()
        self.initUI()
        #sim.simxFinish(-1) #clean all ports

    def initUI(self):
        self.master.title("uArm Robot control")
        # 這裡需要配合各 joint 的轉動範圍角度調整 slider 移動範圍
        for i in range(3):
            tit = "Joint"+str(i+1)
            w_label = Label(self.master, text=tit).grid(row=i, column=0, pady=4, padx = 4)
            win = Scale(self.master, from_=-180, to=180, tickinterval= 30, orient=HORIZONTAL, resolution=1, length=450, command=lambda value, name=i: self.onScale(name, value))
            win.set(0)
            win.grid(row=i, column=1)

            self.scales_l.append(win)
            self.labels_l.append(w_label)

        self.conectar = Button(self.master, text ="To connect", command = self.onConnect)
        self.conectar.grid(row=8,column=1)

    def onScale(self, name,value):
        if(self.clientID != -1):
            print("{} : {}".format(name, value))
            obj = self.handles[int(name)]
            ang = float(value)*math.pi/180
            sim.simxSetJointTargetPosition(self.clientID, obj ,ang ,sim.simx_opmode_oneshot)
            #print(self.objFinal)
            #pos = sim.simxGetObjectPosition(self.clientID, self.objFinal, -1, sim.simx_opmode_streaming )
            #print(pos)



    def onConnect(self):
        if self.clientID != -1:
            return
            
        self.clientID = sim.simxStart('127.0.0.1',19999,True,True,5000,5) # Connect to CoppeliaSim
        if self.clientID == -1:
            messagebox.showinfo( "Can not be connected")
        else:

            for i in range(3):
                j = i + 1
                nom = self.base + str(j)
                _, handle=sim.simxGetObjectHandle(self.clientID, nom, sim.simx_opmode_oneshot_wait)
                self.handles.append(handle)
            self.objFinal = sim.simxGetObjectHandle(self.clientID, 'joint3', sim.simx_opmode_oneshot_wait)
            print(self.objFinal)
            time.sleep(1)
            print("Connected and send command to the motor")

def close_window():
  print("Close all ports")
  sim.simxFinish(-1) #clean all ports
  root.destroy()

def main():
    root.protocol("WM_DELETE_WINDOW", close_window)
    ex = GUI()
    root.geometry("500x500")
    root.mainloop()


if __name__ == '__main__':
    main()
