import React from 'react';
import ReactDOM from 'react-dom/client';
import {BrowserRouter, Route, Routes} from 'react-router-dom'
import './index.css';
import App, { SettingsMidi } from './App';

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    <BrowserRouter>
        <Routes>
            <Route path='/' element={<App/>}/>
            <Route path='/midi' element={<SettingsMidi/>}/>
        </Routes>
  </BrowserRouter>
  </React.StrictMode>
);