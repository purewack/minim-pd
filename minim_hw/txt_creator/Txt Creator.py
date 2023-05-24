# Imports
import tkinter as tk
from tkinter import messagebox, font
from PIL import Image, ImageTk
import numpy as np

class TxtCreator:
    def __init__(self,xPx,yPx):
        self.root_window = tk.Tk()
        self.root_window.iconbitmap("Motif M.ico")
        self.root_window.title("Txt Creator")
        self.screen_size = (self.root_window.winfo_screenwidth(),self.root_window.winfo_screenheight())

        self.taskbar_height = 40
        self.titlebar_height = 31

        self.font = tk.font.Font(font = ("arial",-12,"normal"))

        self.screen_size_no_taskbar = (self.screen_size[0],self.screen_size[1]-self.taskbar_height)

        self.midpoint = (int(self.screen_size[0]/2),int(self.screen_size_no_taskbar[1]/2))

        self.root_window_size = (770,188)
        self.root_window_pos = (int(self.midpoint[0]-self.root_window_size[0]/2),int(self.midpoint[1]-self.root_window_size[0]/2-self.taskbar_height))
        self.root_window.geometry(f"{self.root_window_size[0]}x{self.root_window_size[1]}+{self.root_window_pos[0]}+{self.root_window_pos[1]}")
        self.root_window_interior = (int(self.root_window_size[0]*9/10),int(self.root_window_size[1]*9/10))

        self.spacer_before = tk.Frame(self.root_window, height=14)
        self.spacer_before.pack()

        self.main_frame = tk.Frame(self.root_window, height=100)
        self.main_frame.pack(side="top")

        self.spacer_after = tk.Frame(self.root_window, height=14)
        self.spacer_after.pack()

        self.hz_spacer1 = tk.Frame(self.main_frame, width=65)
        self.hz_spacer1.pack(side="left")

        self.left_main_frame = tk.Frame(self.main_frame, width=150)
        self.left_main_frame.pack(side="left")

        self.hz_spacer2 = tk.Frame(self.main_frame, width=80)
        self.hz_spacer2.pack(side="left")

        self.mid_main_frame = tk.Frame(self.main_frame, width=150)
        self.mid_main_frame.pack(side="left")

        self.hz_spacer3 = tk.Frame(self.main_frame, width=80)
        self.hz_spacer3.pack(side="left")

        self.right_main_frame = tk.Frame(self.main_frame, width=150)
        self.right_main_frame.pack(side="left")

        self.hz_spacer2 = tk.Frame(self.main_frame, width=80)
        self.hz_spacer2.pack(side="left")


        self.open_Toplevel_button = tk.Button(
            font = self.font,
            master = self.left_main_frame,
            text = "Open draw window.",
            width = 150,
            height = 150,
            bg = "#888888",
            fg = "#FFFFFF",
            image = tk.PhotoImage(width = 1, height = 1),
            compound = "c"
        )

        self.open_Toplevel_button.bind("<Button 1>", self.open_Toplevel)
        self.open_Toplevel_button.pack(side="left")

        self.Toplevel_isOpen = False

        self.save_button = tk.Button(
            font = self.font,
            master = self.mid_main_frame,
            text = "Save and output to .txt",
            width = 150,
            height = 150,
            bg = "#888888",
            fg = "#FFFFFF",
            image = tk.PhotoImage(width = 1, height = 1),
            compound = "c"
        )

        self.save_button.bind("<Button 1>", self.save_drawing)
        self.save_button.pack(side="left")

        self.destroy_button = tk.Button(
            font = self.font,
            master = self.right_main_frame,
            text = "Close the draw window.",
            width = 150,
            height = 150,
            bg = "#888888",
            fg = "#FFFFFF",
            image = tk.PhotoImage(width = 1, height = 1),
            compound = "c"
        )

        self.destroy_button.bind("<Button 1>", self.destroy_draw_window)
        self.destroy_button.pack(side="left")

        self.x_px = xPx*8
        self.y_px = yPx*8


    def on_closing(self):
        if messagebox.askokcancel("Quit", "Do you want to close the window?\nAny unsaved work will be lost."):
            self.Toplevel.destroy()
            self.Toplevel_isOpen = False

    def open_Toplevel(self,some_input):
        if not self.Toplevel_isOpen:
            self.Toplevel = tk.Toplevel(self.root_window)
            self.Toplevel.iconbitmap("Motif M.ico")
            self.Toplevel.title("Draw Window")
            toplevel_size = (self.x_px+2,self.y_px+2)
            toplevel_pos = (int(self.midpoint[0]-toplevel_size[0]/2),int(self.midpoint[1]-self.root_window_size[1]/2-self.taskbar_height))
            self.Toplevel.geometry(f"{toplevel_size[0]}x{toplevel_size[1]}+{toplevel_pos[0]}+{toplevel_pos[1]}")
            self.Toplevel_isOpen = True

            self.canvas = tk.Canvas(self.Toplevel, width = self.x_px, height = self.y_px)
            self.img_array = np.zeros((self.y_px,self.x_px))
            for i in range(len(self.img_array)):
                for j in range(len(self.img_array[i])):
                    if (i%8==0) or (j%8==0):
                        self.img_array[i][j] = 255
            self.img = ImageTk.PhotoImage(Image.fromarray(self.img_array))
            self.canvas.create_image(0,0,anchor="nw",image=self.img)
            #self.canvas.bind("<Button-1>",self.on_click)
            self.canvas.bind("<B1-Motion>",self.on_leftclick)
            self.canvas.bind("<B3-Motion>",self.on_rightclick)
            self.canvas.pack()

        self.Toplevel.protocol("WM_DELETE_WINDOW", self.on_closing)

    def on_leftclick(self,event):
        if (min([event.x,event.y]) < 0) or (max([event.x,event.y]) >= 512) or (event.x%8==0) or (event.y%8==0):
            pass
        else:
            i = int(event.y//8)*8+1
            j = int(event.x//8)*8+1
            if self.img_array[i][j] == 0:
                for m in range(7):
                    for n in range(7):
                        self.img_array[i+m][j+n] = 255
                self.img = ImageTk.PhotoImage(Image.fromarray(self.img_array))
                self.canvas.imgref = self.img
                self.canvas.create_image(0,0,anchor="nw",image=self.img)
                self.canvas.update()

    def on_rightclick(self,event):
        if (min([event.x,event.y]) < 0) or (max([event.x,event.y]) >= 512) or (event.x%8==0) or (event.y%8==0):
            pass
        else:
            i = int(event.y//8)*8+1
            j = int(event.x//8)*8+1
            if self.img_array[i][j] == 255:
                for m in range(7):
                    for n in range(7):
                        self.img_array[i+m][j+n] = 0
                self.img = ImageTk.PhotoImage(Image.fromarray(self.img_array))
                self.canvas.imgref = self.img
                self.canvas.create_image(0,0,anchor="nw",image=self.img)
                self.canvas.update()

    def save_drawing(self,some_input):
        if self.Toplevel_isOpen:
            if messagebox.askokcancel("Save","Saving this image to .txt will overwrite previous outputs with unchanged filenames.\nSave anyway?"):
                with open("TxtCreatorOutput.txt", "w") as output_file:
                    for i in range(self.y_px):
                        if i%8==1:
                            for j in range(self.x_px):
                                if j%8==1:
                                    if self.img_array[i][j] == 255:
                                        output_file.write("#")
                                    else:
                                        output_file.write(" ")
                            output_file.write("\n")
        else:
            pass

    def destroy_draw_window(self,some_input):
        if self.Toplevel_isOpen:
            self.on_closing()

        else:
            pass

    def main(self):
        self.root_window.mainloop()

if __name__ == "__main__":
    TxtCreator(64,64).main()