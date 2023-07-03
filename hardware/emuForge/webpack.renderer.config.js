const rules = require('./webpack.rules');
// const CopyPlugin = require("copy-webpack-plugin");

rules.push({
  test: /\.css$/,
  use: [{ loader: 'style-loader' }, { loader: 'css-loader' }],
});

module.exports = {
  // Put your normal webpack config below here
  module: {
    rules,
  },
  // plugins: [
  //   new CopyPlugin({patterns:[
  //     "native/build/Release/minim.node",
  //   ]}),
  // ],
  // target: "node",
  // node: {
  //   __dirname: false,
  // },
  // module: {
  //   rules: [
  //     {
  //       test: /\.node$/,
  //       loader: "node-loader",
  //     },
  //   ],
  // },
};
