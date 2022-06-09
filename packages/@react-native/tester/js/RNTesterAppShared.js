/**
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

import {
  View,
  LogBox,
  Text,
} from 'react-native';
import * as React from 'react';

// RNTester App currently uses AsyncStorage from react-native for storing navigation state
// and bookmark items.
// TODO: Vendor AsyncStorage or create our own.
LogBox.ignoreLogs([/AsyncStorage has been extracted from react-native/, /codegenNativeComponent/]);

const RNTesterApp = (): React.Node => {
  return (
    <View style={{flexDirection: 'row'}}>
      <View style={{maxWidth: '60%', backgroundColor: 'green'}}>
        <Text style={{fontSize: 15}}>
          lorem ipsum dolor lorem ipsum dolor lorem ipsum dolor lorem ipsum dolor
        </Text>
      </View>
    </View>
  );
};

export default RNTesterApp;
