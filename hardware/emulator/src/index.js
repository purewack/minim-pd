import React from 'react';
import ReactDOM from 'react-dom/client';
import {BrowserRouter, Route, Routes} from 'react-router-dom'
import App from './App';
import SettingsMidi from './MidiPrefs';
import './style/theme.css'

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    <App />
    {/* <BrowserRouter>
        <Routes>
            <Route path='/' element={<App/>}/>
            <Route path='/midi' element={<SettingsMidi/>}/>
        </Routes>
  </BrowserRouter> */}
  </React.StrictMode>
);