const path = require('path');
const webpack = require('webpack');

const config = {
  entry: {
    'main/index': './src/main/index.js',
    'renderer/index': './src/renderer/index.js'
  },
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: '[name].js'
  },
  module: {
    rules: [
      {
        test: /\.jsx?$/,
        exclude: /node_modules/,
        use: {
          loader: 'babel-loader',
          options: {
            presets: [
              [
                '@babel/preset-env',
                {
                  targets: '>1%, not ie 11, not op_mini all'
                }
              ],
              '@babel/preset-react'
            ],
            plugins: ['@babel/plugin-proposal-class-properties']
          }
        }
      }
    ]
  },
  node: {
    fs: false
  },
  plugins: [
    new webpack.NormalModuleReplacementPlugin(
      /^fs$/,
      path.resolve(__dirname, 'src/fs.js')
    )
  ]
};

if (process.env.NODE_ENV === 'production') {
  config.mode = 'production';
} else {
  config.mode = 'development';
  config.devtool = '#cheap-module-source-map';
  config.plugins.push(new webpack.HotModuleReplacementPlugin());
  config.devServer = {
    contentBase: path.resolve(__dirname, 'dist'),
    hot: true
  };
}

module.exports = config;
