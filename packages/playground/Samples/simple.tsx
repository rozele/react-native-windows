/**
 * Copyright (c) Microsoft Corporation.
 * Licensed under the MIT License.
 * @format
 */
import React from 'react';
import {AppRegistry, requireNativeComponent} from 'react-native';

const TestView: any = requireNativeComponent("TestView");

export default class Bootstrap extends React.Component {
  render() {
    return (
      <TestView backgroundColor='red' />
    );
  }
}

AppRegistry.registerComponent('Bootstrap', () => Bootstrap);
