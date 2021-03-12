/**
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * @format
 * @flow
 */

'use strict';

const React = require('react');

const {
  ScrollView,
  StyleSheet,
  Text,
  TouchableOpacity,
} = require('react-native');

const NUM_ITEMS = 20;

class ScrollViewSimpleExample extends React.Component<{...}> {
  constructor(props) {
    super(props);
    this.state = {head: 0, tail: NUM_ITEMS};
  }

  makeItems: (nItems: number, isHead: boolean, styles: any) => Array<any> = (
    nItems: number,
    isHead: boolean,
    styles,
  ): Array<any> => {
    const items = [];
    for (let i = 0; i < nItems; i++) {
      const isEven = i % 2 === 0;
      const key = isHead ? -(i + 1) : i;
      items[i] = (
        <TouchableOpacity
          key={key}
          style={styles}
          onPress={() =>
            isEven
              ? this.setState({head: this.state.head + 1})
              : this.setState({tail: this.state.tail + 1})
          }>
          <Text>{`Item ${key} (${isEven ? 'Add Above' : 'Add Below'})`}</Text>
        </TouchableOpacity>
      );
    }
    return items;
  };

  render(): React.Node {
    // One of the items is a horizontal scroll view
    const head = this.makeItems(
      this.state.head,
      true,
      styles.itemWrapper,
    ).reverse();
    const tail = this.makeItems(this.state.tail, false, styles.itemWrapper);

    const verticalScrollView = (
      <ScrollView nativeInverted style={styles.verticalScrollView}>
        {[...head, ...tail]}
      </ScrollView>
    );

    return verticalScrollView;
  }
}

const styles = StyleSheet.create({
  verticalScrollView: {
    margin: 10,
    marginBottom: 60,
  },
  itemWrapper: {
    backgroundColor: '#dddddd',
    alignItems: 'center',
    borderRadius: 5,
    borderWidth: 5,
    borderColor: '#a52a2a',
    padding: 30,
    margin: 5,
  },
  horizontalItemWrapper: {
    padding: 50,
  },
  horizontalPagingItemWrapper: {
    width: 200,
  },
});

exports.title = 'ScrollViewSimpleExample';
exports.category = 'Basic';
exports.description =
  'Component that enables scrolling through child components.';

exports.examples = [
  {
    title: 'Simple scroll view',
    render: function(): React.Element<typeof ScrollViewSimpleExample> {
      return <ScrollViewSimpleExample />;
    },
  },
];
