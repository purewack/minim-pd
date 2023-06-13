const { ipcRenderer, contextBridge } = require('electron');

contextBridge.exposeInMainWorld('contexts', {
    openMIDIIO: () => {
        ipcRenderer.send('openMIDIIO')
    },
});