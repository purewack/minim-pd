import React from 'react';
import ReactDOM from 'react-dom/client';
// import {BrowserRouter, Route, Routes} from 'react-router-dom'
import App from './App';
import './style/theme.css'

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    {/* <h1>Hello</h1> */}
    <App />
    {/* <BrowserRouter>
        <Routes>
            <Route path='/' element={<App/>}/>
            <Route path='/midi' element={<SettingsMidi/>}/>
        </Routes>
  </BrowserRouter> */}
  </React.StrictMode>
);