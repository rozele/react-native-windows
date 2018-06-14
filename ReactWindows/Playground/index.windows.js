/**
 * Sample React Native App
 * https://github.com/facebook/react-native
 */

import React, { Component } from 'react';
import {
  AppRegistry,
  Button,
  NativeModules,
  StyleSheet,
  Text,
  View,
  requireNativeComponent,
} from 'react-native';

var PropTypes = require('prop-types');

class TestView extends Component {

  componentWillUnmount() {
    NativeModules.TestModule.sendEvent();
  }

  onTest() {
    alert('Received event!');
  }

  render() {
    return <NativeTestView onTest={this.onTest} />
  }
}

NativeTestView = requireNativeComponent(
  'TestView',
  TestView,
  {
    nativeOnly: {
      onTest: true,
    },
  });

class Playground extends Component {
  state = {
    toggled: false,
  }

  render() {
    let test = null;
    if (this.state.toggled) {
      test = (
        <TestView />
      );
    }

    return (
      <View style={styles.container}>
        <Text style={styles.welcome}>
          Welcome to React Native!
        </Text>
        <Text style={styles.instructions}>
          To get started, edit index.windows.js
        </Text>
        <Text style={styles.instructions}>
          Shake or press Shift+F10 for dev menu
        </Text>
        <Button 
          title="Toggle" 
          onPress={() => this.setState({toggled: !this.state.toggled})} />
        {test}
      </View>
    );
  }
}

const styles = StyleSheet.create({
  container: {
    flex: 1,
    justifyContent: 'center',
    alignItems: 'center',
    backgroundColor: '#F5FCFF',
  },
  welcome: {
    fontSize: 20,
    textAlign: 'center',
    margin: 10,
  },
  instructions: {
    textAlign: 'center',
    color: '#333333',
    marginBottom: 5,
  },
});

AppRegistry.registerComponent('Playground', () => Playground);
