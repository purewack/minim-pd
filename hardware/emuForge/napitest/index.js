console.log('binding', __dirname)
const addon = require('node-gyp-build')(__dirname)
console.log(addon)
module.exports = addon
