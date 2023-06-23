const path = require('path');
const { app, BrowserWindow, ipcMain } = require('electron');
const isDev = require('electron-is-dev');

let win
function createWindow() {
  // Create the browser window.
  win = new BrowserWindow({
    title:'MINIM Emulator App',
    width: 750,
    height: 460,
    frame: false,
    webPreferences: {
      contextIsolation: true,
      nodeIntegration: true,
      preload: path.join(__dirname, "preload.js"),
   },
  });
  win.loadURL(
    isDev
      ? 'http://localhost:3000'
      : `file://${path.join(__dirname, '../build/index.html')}`
  );

  // Open the DevTools.
  // if (isDev) {
  //   win.webContents.openDevTools({ mode: 'detach' });
  // }

  // ipcMain.on('openMIDIIO', 
  //   ()=>{
  //     const win = new BrowserWindow({
  //       title:'MIDI Prefs',
  //       width: 400,
  //       height: 400,
  //     });
  //     win.loadURL(
  //       isDev
  //         ? 'http://localhost:3000/midi'
  //         : `file://${path.join(__dirname, '../build/index.html/midi')}`
  //     );
  //   }
}


ipcMain.on('end',()=>{
  app.quit();
})
ipcMain.on('dev',()=>{
  win.webContents.openDevTools({ mode: 'detach' });
})
// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.whenReady().then(createWindow);

// Quit when all windows are closed, except on macOS. There, it's common
// for applications and their menu bars to stay active until the user quits
// explicitly with Cmd + Q.
app.on('window-all-closed', () => {
  // if (process.platform !== 'darwin') {
    app.quit();
  // }
});

app.on('activate', () => {
  if (BrowserWindow.getAllWindows().length === 0) {
    createWindow();
  }
});

app.on('uncaughtException', function (err) {
  console.log('uncaughtException',err);
})