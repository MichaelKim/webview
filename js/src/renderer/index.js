import React from 'react';
import ReactDOM from 'react-dom';
import fs from 'fs';

class App extends React.Component {
  render() {
    return (
      <div>
        <p>Hello world!</p>
        <p>{fs(1)}</p>
      </div>
    );
  }
}

ReactDOM.render(<App />, document.getElementById('app'));
